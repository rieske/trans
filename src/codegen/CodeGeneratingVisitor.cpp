#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "Instruction.h"

#include <map>
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
#include "types/IntegerConstant.h"
#include "types/ObjectAbiType.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/ImmediateFormat.h"

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

void CodeGeneratingVisitor::emitBooleanConvert(const std::string& sourceName,
        const std::string& destName) {
    const std::string one = "__bc" + std::to_string(convertLabel_++) + "t";
    const std::string done = "__bc" + std::to_string(convertLabel_++) + "d";
    emit(ir::zeroCompare(sourceName));
    emit(ir::jump(one, JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant("0", destName));
    emit(ir::jump(done));
    emit(ir::label(one));
    emit(ir::assignConstant("1", destName));
    emit(ir::label(done));
}

void CodeGeneratingVisitor::emitConvert(const std::string& sourceName, const std::string& destName,
        const type::Type& sourceType, const type::Type& destType) {
    if (type::needsBoolConvert(sourceType, destType)) {
        emitBooleanConvert(sourceName, destName);
        return;
    }
    if (type::needsIntegerWiden(sourceType, destType)
            || type::needsIntegerToPointerExtend(sourceType, destType)) {
        emit(ir::widen(sourceName, destName, type::valueIsSigned(sourceType)));
        return;
    }
    emit(ir::assign(sourceName, destName));
}

void CodeGeneratingVisitor::emitIntegerConstant(const type::IntegerConstant& value,
        const std::string& dest) {
    const std::string lo = util::wordImmediate(type::bitsWord(value, 0));
    if (type::object_abi::valueWords(value.type.getSize()) > 1) {
        emit(ir::assignConstant(lo, util::wordImmediate(type::bitsWord(value, 1)), dest));
        return;
    }
    emit(ir::assignConstant(lo, dest));
}

std::string CodeGeneratingVisitor::materializeConversion(ast::Expression& expression) {
    if (!expression.hasResultSymbol(store_)) {
        return {};
    }
    if (auto* conv = store_.value(&expression, symbols::ValueSlot::Conversion)) {
        emitConvert(expression.getResultSymbol(store_)->getName(), conv->getName(),
                expression.getResultSymbol(store_)->getType(), conv->getType());
        return conv->getName();
    }
    return expression.getResultSymbol(store_)->getName();
}

std::string CodeGeneratingVisitor::generateExpression(ast::Expression& expression) {
    expression.accept(*this);
    materializeFieldIndexLoad(expression, *this, store_);
    materializeArrayDecay(expression, *this, store_);
    return materializeConversion(expression);
}

void CodeGeneratingVisitor::emitAddressOf(ast::Expression& operand, const std::string& destName) {
    // Plan is produced by SA on unary & (keyed by operand). Exhaustive std::visit.
    // Fail-closed: missing plan is incomplete SA or a bug - do not invent ResultAddressOf.
    const symbols::AddressPlan* planPtr = store_.addressPlan(&operand);
    if (!planPtr) {
        throw std::runtime_error { "AddressOf missing AddressPlan from SA" };
    }

    std::visit(
            [&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::FieldPlan>
                        || std::is_same_v<T, symbols::IndexPlan>
                        || std::is_same_v<T, symbols::LvaluePlan>) {
                    operand.accept(*this);
                    emit(ir::assign(
                            operand.getLvalueSymbol(store_)->getName(), destName));
                } else if constexpr (std::is_same_v<T, symbols::FunctionDesignatorPlan>) {
                    generateExpression(operand);
                    emit(ir::assign(
                            operand.getResultSymbol(store_)->getName(), destName));
                } else if constexpr (std::is_same_v<T, symbols::ResultAddressOfPlan>) {
                    generateExpression(operand);
                    emitArrayObjectAddress(
                            *operand.getResultSymbol(store_), destName);
                } else if constexpr (std::is_same_v<T, symbols::ArrayDecayPlan>) {
                    throw std::runtime_error { "AddressOf saw ArrayDecayPlan" };
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
    auto* holder = store_.value(&declarator, symbols::ValueSlot::Holder);
    // File-scope variables are initialized in .data; skip children (would emit assigns with no procedure).
    if (declarator.hasInitializer() && holder && holder->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (holder && !holder->isGlobal() && type::hasComputableRuntimeSize(holder->getType())) {
        const std::string sizeName = addScratchValue(type::signedInteger());
        emitSizeofProduct(holder->getType(), sizeName);
        emit(ir::allocaBytes(sizeName, holder->getName()));
    }
    if (!store_.fieldInits(&declarator).empty()) {
        // visitChildren only forms Field/Index addresses; load a struct copy
        // source such as `struct T x = p->m` before storing.
        if (declarator.hasInitializer()) {
            if (ast::Expression* init = declarator.getInitializer()) {
                materializeFieldIndexLoad(*init, *this, store_);
            }
        }
        emitFieldInits(*this, holder->getName(), store_.fieldInits(&declarator));
        return;
    }
    if (declarator.hasInitializer()) {
        ast::Expression* init = ast::effectiveInitializer(
                holder->getType(), declarator.getInitializer());
        // Aggregate brace lists are handled via FieldInit above; anything still
        // a list has no scalar value to assign.
        if (dynamic_cast<ast::InitializerListExpression*>(init)) {
            return;
        }
        if (!init || !init->hasResultSymbol(store_)) {
            return;
        }
        // visitChildren already visited init; only materialize delayed load/decay/conversion.
        materializeFieldIndexLoad(*init, *this, store_);
        materializeArrayDecay(*init, *this, store_);
        emit(ir::assign(materializeConversion(*init), holder->getName()));
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
    // Size is folded in semantic analysis; visiting the bound would emit into no procedure
    // for file-scope prototypes such as `char[20]`.
    (void)declaration;
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}


void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    // No functionFrame/symbol when the definition is invalid (e.g. name conflicts).
    if (const auto* frame = store_.functionFrameIfAny(&function); !frame || !frame->symbol) {
        return;
    }

    function.visitDeclarator(*this);

    auto instructionsBak = std::move(instructions);
    instructions.clear();
    auto* localsBak = currentLocals_;
    auto locals = store_.functionFrame(&function).locals;
    currentLocals_ = &locals;
    function.visitBody(*this);
    currentLocals_ = localsBak;

    std::vector<Value> arguments;
    for (auto& argumentSymbol : store_.functionFrame(&function).arguments) {
        // Full type::Type kept on FrameSymbol; size/signedness/kind via TypeQuery.
        arguments.push_back(frameSymbolFrom(argumentSymbol).toValue());
    }
    const bool variadic = store_.functionFrame(&function).symbol.get()->getType().isVariadic();
    const type::Type& retType = store_.functionFrame(&function).symbol.get()->returnType();
    const bool memoryReturn = type::object_abi::typeNeedsMemoryReturn(retType);

    Procedure procedure;
    procedure.name = store_.functionFrame(&function).symbol.get()->getName();
    procedure.frame.arguments = std::move(arguments);
    procedure.variadic = variadic;
    procedure.exported = !store_.functionFrame(&function).symbol.get()->hasInternalLinkage();
    procedure.memoryReturn = memoryReturn;
    procedure.body = std::move(instructions);
    localsByProcedure_[procedure.name] = std::move(locals);
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
