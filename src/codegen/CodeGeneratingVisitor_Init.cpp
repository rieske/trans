#include "CodeGeneratingVisitor.h"

#include <cassert>

#include "Instruction.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "symbols/AddressPlan.h"

namespace codegen {

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    auto* holder = declarator.getHolder(store_);
    // .data init; visiting children would emit assigns with no procedure.
    if (declarator.hasInitializer() && holder && holder->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (!declarator.hasInitializer()) {
        return;
    }
    assert(holder && "InitializedDeclarator holder required after successful SA");
    const auto& fieldStores = store_.structFieldInits(&declarator);
    if (!fieldStores.empty()) {
        emitStructFieldInits(holder->getName(), fieldStores);
        return;
    }
    if (declarator.getInitializer()->hasResultSymbol(store_)) {
        emit(ir::assign(
                convertedResultName(*declarator.getInitializer()), holder->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::CompoundLiteral& expression) {
    expression.initializer().accept(*this);
    auto* object = objectHome(expression);
    if (!object) {
        return;
    }
    const auto& fieldStores = store_.structFieldInits(&expression);
    if (!fieldStores.empty()) {
        emitStructFieldInits(object->getName(), fieldStores);
    } else if (expression.initializer().hasResultSymbol(store_)) {
        emit(ir::assign(convertedResultName(expression.initializer()), object->getName()));
    }
}

void CodeGeneratingVisitor::emitStructFieldInits(const std::string& objectName,
        const std::vector<symbols::StructFieldInit>& fieldStores) {
    for (const auto& field : fieldStores) {
        emit(ir::fieldAddress(
                objectName, field.offsetBytes, field.addressName,
                symbols::AddressBaseMode::LeaObject));
        if (field.zeroInitialize) {
            emit(ir::assignConstant("0", field.sourceName));
        }
        if (field.isBitField()) {
            emitBitFieldInsert(field.addressName, field.sourceName, *field.bitField, field.type);
        } else {
            emit(ir::lvalueAssign(field.sourceName, field.addressName));
        }
    }
}

} // namespace codegen
