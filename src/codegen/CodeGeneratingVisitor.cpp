#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "Instruction.h"

#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "FrameLayout.h"
#include "FrameSymbol.h"
#include "IrPasses.h"
#include "ast/Expression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/EffectiveInitializer.h"
#include "ast/InitializerListExpression.h"
#include "symbols/ValueEntry.h"
#include "types/ObjectAbiType.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

namespace codegen {




CodeGeneratingVisitor::CodeGeneratingVisitor(symbols::AnnotationStore& store)
    : store_ { store } {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}



void CodeGeneratingVisitor::narrowIntegralResult(const type::Type& resultType, const std::string& resultName) {
    if (resultType.kind() != type::TypeKind::Primitive || type::isFloating(resultType)) {
        return;
    }
    const int size = resultType.getSize();
    if (size > 0 && size < 8) {
        emit(ir::truncate(
                resultName, size, type::valueIsSigned(resultType)));
    }
}


std::string CodeGeneratingVisitor::generateExpression(ast::Expression& expression) {
    expression.accept(*this);
    materializeArrayDecay(expression, *this, store_);
    if (const std::string* convertTo = store_.string(&expression, symbols::StringSlot::ConversionTarget)) {
        emit(ir::assign(
                expression.result(store_)->getName(), *convertTo));
        return *convertTo;
    }
    if (!expression.hasResult(store_)) {
        return {};
    }
    return expression.result(store_)->getName();
}

void CodeGeneratingVisitor::emitAddressOf(ast::Expression& operand, const std::string& destName) {
    // Plan is produced by SA on unary & (keyed by operand). Exhaustive std::visit.
    // Fail-closed: missing plan is incomplete SA or a bug — do not invent ResultAddressOf.
    const symbols::AddressPlan* planPtr = store_.addressPlan(&operand);
    if (!planPtr) {
        throw std::runtime_error { "AddressOf missing AddressPlan from SA" };
    }

    std::visit(
            [&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::FieldPlan>) {
                    auto* base = arm.baseExpr.template as<ast::Expression>();
                    generateExpression(*base);
                    emitFieldFromPlan(arm, destName, *this);
                } else if constexpr (std::is_same_v<T, symbols::IndexPlan>) {
                    auto* base = arm.baseExpr.template as<ast::Expression>();
                    auto* index = arm.indexExpr.template as<ast::Expression>();
                    generateExpression(*base);
                    generateExpression(*index);
                    emitIndexFromPlan(arm, index->result(store_)->getName(), destName, *this);
                } else if constexpr (std::is_same_v<T, symbols::FunctionDesignatorPlan>) {
                    generateExpression(operand);
                    emit(ir::assign(
                            operand.result(store_)->getName(), destName));
                } else if constexpr (std::is_same_v<T, symbols::LvaluePlan>) {
                    generateExpression(operand);
                    emit(ir::assign(
                            operand.lvalueAnnotation(store_)->getName(), destName));
                } else if constexpr (std::is_same_v<T, symbols::ResultAddressOfPlan>) {
                    generateExpression(operand);
                    emit(ir::addressOf(
                            operand.result(store_)->getName(), destName));
                }
            },
            *planPtr);
}

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers&) {
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    // File-scope variables are initialized in .data; skip children (would emit assigns with no procedure).
    if (declarator.hasInitializer() && store_.value(&declarator, symbols::ValueSlot::Holder)->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (!store_.structFieldInits(&declarator).empty()) {
        // Leaf expressions are evaluated via visit(InitializerListExpression) recursion
        // from visitChildren; only emit field stores here.
        emitStructFieldInits(*this, store_.value(&declarator, symbols::ValueSlot::Holder)->getName(),
                store_.structFieldInits(&declarator));
        return;
    }
    if (declarator.hasInitializer()) {
        auto* holder = store_.value(&declarator, symbols::ValueSlot::Holder);
        ast::Expression* init = ast::effectiveInitializer(
                holder->getType(), declarator.getInitializer());
        // Aggregate brace lists are handled via StructFieldInit above; anything still
        // a list has no scalar value to assign.
        if (dynamic_cast<ast::InitializerListExpression*>(init)) {
            return;
        }
        if (!init || !init->hasResult(store_)) {
            return;
        }
        // visitChildren already visited init; only materialize delayed decay/conversion.
        materializeArrayDecay(*init, *this, store_);
        std::string valueName = init->result(store_)->getName();
        if (const std::string* convertTo = store_.string(init, symbols::StringSlot::ConversionTarget)) {
            emit(ir::assign(valueName, *convertTo));
            valueName = *convertTo;
        }
        // If still array-typed (no semantic decay), form &arr for pointer holders.
        if (holder->getType().isPointer() && init->hasResult(store_)
                && init->result(store_)->getType().isArray()) {
            if (auto* addr = init->lvalueAnnotation(store_)) {
                emit(ir::assign(addr->getName(), holder->getName()));
            } else {
                emit(ir::addressOf(
                        init->result(store_)->getName(), holder->getName()));
            }
            return;
        }
        emit(ir::assign(valueName, holder->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclarator& declaration) {
    // Array bounds are constant-folded when computing the type; do not emit code for them
    // (parameter arrays would otherwise produce assigns outside any procedure).
    (void)declaration;
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}


void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    // Semantic analysis skips setSymbol when the definition is invalid (e.g. name conflicts).
    if (const auto* frame = store_.functionFrameIfAny(&function); !frame || !frame->symbol) {
        return;
    }

    function.visitDeclarator(*this);

    auto instructionsBak = std::move(instructions);
    instructions.clear();
    function.visitBody(*this);

    std::vector<Value> arguments;
    for (auto& argumentSymbol : store_.functionFrame(&function).arguments) {
        // Full type::Type kept on FrameSymbol; size/signedness/kind via TypeQuery.
        arguments.push_back(frameSymbolFrom(argumentSymbol).toValue());
    }
    const bool variadic = store_.functionFrame(&function).symbol.get()->getType().isVariadic();
    bool isStatic = false;
    for (const auto& storage : function.getReturnTypeSpecifiers().getStorageSpecifiers()) {
        if (storage.getStorage() == ast::Storage::STATIC) {
            isStatic = true;
            break;
        }
    }
    // System V memory return for aggregates > 16 bytes (git strbuf, config store_create_section).
    // Same predicate as Call sites (variadic suppresses sret on both sides).
    const type::Type& retType = store_.functionFrame(&function).symbol.get()->returnType();
    const bool memoryReturn = type::object_abi::productEmitsMemoryReturn(retType, variadic);

    Procedure procedure;
    procedure.name = store_.functionFrame(&function).symbol.get()->getName();
    procedure.frame.arguments = std::move(arguments);
    procedure.variadic = variadic;
    procedure.isStatic = isStatic;
    procedure.memoryReturn = memoryReturn;
    procedure.body = std::move(instructions);
    localsByProcedure_[procedure.name] = store_.functionFrame(&function).locals;
    module_.procedures.push_back(std::move(procedure));
    instructions = std::move(instructionsBak);
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

void CodeGeneratingVisitor::emit(Instruction instruction) {
    instructions.push_back(std::move(instruction));
}

void CodeGeneratingVisitor::packFrames(IntermediateRepresentation& ir) {
    for (auto& procedure : ir.procedures) {
        auto it = localsByProcedure_.find(procedure.name);
        if (it == localsByProcedure_.end()) {
            throw std::logic_error { "packFrames: missing locals for procedure " + procedure.name };
        }
        procedure.frame.locals = packFrameValues(it->second, procedure.body);
    }
}

IntermediateRepresentation CodeGeneratingVisitor::takeFinishedIr() {
    IntermediateRepresentation ir = runIrPasses(std::move(module_));
    packFrames(ir);
    return ir;
}



} // namespace codegen
