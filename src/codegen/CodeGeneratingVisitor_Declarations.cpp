#include "CodeGeneratingVisitor.h"
#include "ast/AstNodes.h"
#include "codegen/InternalError.h"
#include "codegen/IrBuilders.h"

#include "symbols/ValueEntry.h"
#include "types/ObjectAbiType.h"
#include "types/TypeQuery.h"

namespace codegen {

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers&) {
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    auto* holder = declarator.getHolder(store_);
    // .data init; visiting children would emit assigns with no procedure.
    if (declarator.hasInitializer() && holder && holder->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (holder && !holder->isGlobal() && type::hasComputableRuntimeSize(holder->getType())) {
        const int sizeName = addScratchValue(type::signedInteger());
        emitSizeofProduct(holder->getType(), sizeName);
        emit(ir::allocaBytes(sizeName, id(*holder)));
    }
    if (!declarator.hasInitializer()) {
        return;
    }
    require(holder, "InitializedDeclarator holder");
    const auto& fieldStores = store_.structFieldInits(&declarator);
    if (!fieldStores.empty()) {
        emitStructFieldInits(id(*holder), fieldStores);
        return;
    }
    if (declarator.getInitializer()->hasResultSymbol(store_)) {
        emit(ir::assign(
                convertedResult(*declarator.getInitializer()), id(*holder)));
    }
}

void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclarator&) {
    // Size is folded in semantic analysis; visiting the bound would emit into no procedure
    // for file-scope prototypes such as `char[20]`.
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}

void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    // SA writes a frame only for a valid definition (e.g. no name conflict).
    const auto* frame = store_.functionFrame(&function);
    if (!frame) {
        return;
    }

    function.visitDeclarator(*this);

    std::vector<Value> values;
    for (const auto& valueSymbol : frame->locals) {
        values.push_back(valueFromSymbol(module_.strings, valueSymbol.second));
    }
    std::vector<Value> arguments;
    for (const auto& argumentSymbol : frame->arguments) {
        arguments.push_back(valueFromSymbol(module_.strings, argumentSymbol));
    }
    Procedure procedure;
    procedure.name = id(frame->symbol.getName());
    procedure.frame.locals = std::move(values);
    procedure.frame.arguments = std::move(arguments);
    const bool variadic = frame->symbol.isVariadic();
    procedure.memoryReturn = type::object_abi::typeNeedsMemoryReturn(
            frame->symbol.returnType());
    procedure.variadic = variadic;
    procedure.exported = frame->symbol.providesExternalDefinition();
    internProcedureTemps(module_.strings, procedure);

    std::vector<Instruction>* previousBody = currentBody_;
    Procedure* previousProcedure = currentProcedure_;
    currentProcedure_ = &procedure;
    currentBody_ = &procedure.body;
    function.visitBody(*this);
    currentProcedure_ = previousProcedure;
    currentBody_ = previousBody;

    module_.procedures.push_back(std::move(procedure));
}

} // namespace codegen
