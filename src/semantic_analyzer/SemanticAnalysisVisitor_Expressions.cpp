#include "SemanticAnalysisVisitorInternal.h"
#include "types/TypeQuery.h"

#include "ast/InitializerListExpression.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    if (!arrayAccess.hasLeftOperandSymbol(annotations()) || !arrayAccess.hasRightOperandSymbol(annotations())) {
        return;
    }

    type::Type exprType = arrayAccess.getLeftOperand()->expressionType();
    type::Type valueType = arrayAccess.getLeftOperand()->valueType(annotations());
    type::ArraySubscriptInfo sub = type::arraySubscriptInfo(exprType, valueType);
    if (!sub.valid()) {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
        return;
    }

    type::Type elementType = sub.elementType;

    symbols::IndexPlan indexPlan;
    indexPlan.baseExpr = arrayAccess.getLeftOperand();
    indexPlan.indexExpr = arrayAccess.getRightOperand();
    indexPlan.elementSize = sub.elementStride;
    indexPlan.baseMode = sub.baseIsArray ? symbols::AddressBaseMode::LeaObject
                                         : symbols::AddressBaseMode::PointerValue;

    if (elementType.isArray() || elementType.isRecord()) {
        type::Type addrType = elementType.isArray()
                ? type::pointer(elementType.getElementType())
                : type::pointer(elementType);
        auto addr = symbolTable.createTemporarySymbol(addrType);
        arrayAccess.setLvalueSymbol(annotations(), addr);
        arrayAccess.setAggregateAddressResult(annotations(), addr, elementType);
    } else {
        auto addr = symbolTable.createTemporarySymbol(type::pointer(elementType));
        arrayAccess.setLvalueSymbol(annotations(), addr);
        arrayAccess.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(elementType));
    }
    annotations().setAddressPlan(&arrayAccess, symbols::AddressPlan { indexPlan });
}

void SemanticAnalysisVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
    // Nested brace lists are applied by lowerLocalInitializer (positional nested/array).
    // A single-element list may act as a scalar brace init { x }.
    if (expression.getElements().size() == 1 && expression.getElements().front().value
            && expression.getElements().front().value->hasResultSymbol(annotations())) {
        expression.setResultSymbol(annotations(),
                *expression.getElements().front().value->getResultSymbol(annotations()));
    }
}

void SemanticAnalysisVisitor::visit(ast::MemberAccess& memberAccess) {
    memberAccess.getBase()->accept(*this);
    if (!memberAccess.getBase()->hasResultSymbol(annotations())) {
        return;
    }
    MemberBaseResolution base = resolveMemberBase(*memberAccess.getBase(), memberAccess.isArrow());
    if (!base.ok) {
        semanticError(base.error, memberAccess.getContext());
        return;
    }

    int offset = 0;
    type::Type memberType = type::voidType();
    if (!base.structureType.memberOffset(memberAccess.getMemberName(), offset)
            || !base.structureType.memberType(memberAccess.getMemberName(), memberType)) {
        semanticError("no member named ‘" + memberAccess.getMemberName() + "’ in structure or union",
                memberAccess.getContext());
        return;
    }
    auto fieldAddr = symbolTable.createTemporarySymbol(type::pointer(memberType));
    memberAccess.setLvalueSymbol(annotations(), fieldAddr);
    symbols::FieldPlan fieldPlan;
    fieldPlan.baseExpr = memberAccess.getBase();
    fieldPlan.fieldOffsetBytes = offset;
    fieldPlan.baseMode = base.addressIsPointer ? symbols::AddressBaseMode::PointerValue
                                               : symbols::AddressBaseMode::LeaObject;
    annotations().setAddressPlan(&memberAccess, symbols::AddressPlan { fieldPlan });
    if (memberType.isRecord() || memberType.isArray()) {
        memberAccess.setAggregateAddressResult(annotations(), fieldAddr, memberType);
    } else {
        memberAccess.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(memberType));
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(constant.getType()));
}

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    stringLiteral.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(stringLiteral.getType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.operandType(), expression.getContext());

    expression.setType(expression.operandType());
    auto operandSymbol = *expression.operandSymbol(annotations());
    expression.setResultSymbol(annotations(), operandSymbol);

    auto preOperationSymbolName = operandSymbol.getName() + "_pre";
    symbolTable.insertSymbol(preOperationSymbolName, operandSymbol.getType(), operandSymbol.getContext());
    expression.setPreOperationSymbol(annotations(), symbolTable.lookup(preOperationSymbolName));

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.operandType(), expression.getContext());

    expression.setType(expression.operandType());
    expression.setResultSymbol(annotations(), *expression.operandSymbol(annotations()));

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    const auto& lexeme = expression.getOperator()->getLexeme();
    if (lexeme == "sizeof") {
        expression.visitOperand(*this);
        if (!expression.hasOperandSymbol(annotations())) {
            expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
            return;
        }
        // C: sizeof does not decay a function designator; it remains incomplete.
        if (expression.getOperandExpression()->holdsFunctionDesignator()) {
            semanticError(
                    "invalid application of ‘sizeof’ to incomplete type ‘function’",
                    expression.getContext());
            expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
            return;
        }
        const type::Type& operandType = expression.operandType();
        // Mirror sizeof(type): void, bare function, and incomplete records are incomplete.
        // Pointers (including pointer-to-function) are complete object types.
        if (type::isIncompleteObjectType(operandType)) {
            semanticError(
                    "invalid application of ‘sizeof’ to incomplete type ‘" + operandType.to_string() + "’",
                    expression.getContext());
            expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
            return;
        }
        expression.setSizeofValue(operandType.getSize());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
        return;
    }

    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol(annotations())) {
        return;
    }

    switch (lexeme.front()) {
    case '&': {
        // &function designator: same pointer-to-function value as bare designator decay (C).
        if (expression.getOperandExpression()->holdsFunctionDesignator()) {
            expression.setResultSymbol(annotations(), *expression.operandSymbol(annotations()));
            break;
        }
        rejectFunctionValue(expression.operandType(), expression.getContext());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
        break;
    }
    case '*': {
        rejectFunctionValue(expression.operandType(), expression.getContext());
        type::Type operandType = expression.operandType();
        const type::Type valueType = expression.operandSymbol(annotations())->getType();
        // Value already a pointer (e.g. multi-dim a[i] decayed row, or int(*)[N]).
        if (valueType.isPointer()) {
            type::Type pointee = valueType.dereference();
            if (type::isBareFunction(pointee)) {
                // *fp for a bare function pointee: keep the pointer value (no memory load).
                // Pointer-to-function pointees still need a load (pointer-to-pointer-to-function).
                expression.setResultSymbol(annotations(), *expression.operandSymbol(annotations()));
            } else if (pointee.isArray()) {
                // *ptr-to-array yields the array object (address); do not scalar-load the row.
                auto addr = symbolTable.createTemporarySymbol(type::pointer(pointee.getElementType()));
                expression.setLvalueSymbol(annotations(), addr);
                expression.setAggregateAddressResult(annotations(), addr, pointee);
            } else {
                expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(pointee));
                expression.setLvalueSymbol(annotations(), symbolTable.createTemporarySymbol(valueType));
            }
            break;
        }
        // Array object in memory: *a ≡ a[0].
        if (operandType.isArray()) {
            type::Type elem = operandType.getElementType();
            if (elem.isArray()) {
                // *a for multi-dim: yield decayed address of first row; keep array expr type.
                auto addr = symbolTable.createTemporarySymbol(type::pointer(elem.getElementType()));
                expression.setLvalueSymbol(annotations(), addr);
                expression.setAggregateAddressResult(annotations(), addr, elem);
            } else {
                auto addr = symbolTable.createTemporarySymbol(type::pointer(elem));
                expression.setLvalueSymbol(annotations(), addr);
                expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(elem));
                expression.setType(elem);
            }
            break;
        }
        semanticError("invalid type argument of ‘unary *’ :" + operandType.to_string(), expression.getContext());
        break;
    }
    case '+':
        rejectFunctionValue(expression.operandType(), expression.getContext());
        expression.setResultSymbol(annotations(), *expression.operandSymbol(annotations()));
        break;
    case '-':
        rejectFunctionValue(expression.operandType(), expression.getContext());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case '~':
        rejectFunctionValue(expression.operandType(), expression.getContext());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case '!':
        rejectFunctionValue(expression.operandType(), expression.getContext());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
        expression.setTruthyLabel(annotations(), symbolTable.newLabel());
        expression.setFalsyLabel(annotations(), symbolTable.newLabel());
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void SemanticAnalysisVisitor::visit(ast::StatementExpression& expression) {
    expression.body().accept(*this);
    const auto& items = expression.body().getItems();
    if (!items.empty()) {
        if (auto* last = dynamic_cast<ast::Expression*>(items.back().get())) {
            if (last->hasResultSymbol(annotations())) {
                expression.takeValueFrom(*last, annotations());
                return;
            }
        }
    }
    expression.setType(type::voidType());
}

void SemanticAnalysisVisitor::visit(ast::GenericSelection& expression) {
    expression.controllingExpression().accept(*this);
    if (!expression.controllingExpression().hasResultSymbol(annotations())
            || !expression.controllingExpression().hasExpressionType()) {
        return;
    }
    const type::Type converted = type::afterLvalueConversion(
            expression.controllingExpression().expressionType());

    std::optional<std::size_t> defaultIndex;
    std::vector<std::size_t> matches;
    auto& associations = expression.associations();
    for (std::size_t i = 0; i < associations.size(); ++i) {
        auto& association = associations[i];
        if (association.typeName) {
            association.typeName->resolveTypeof(*this);
            if (!association.typeName->hasType()) {
                semanticError("cannot determine type of generic association", expression.getContext());
                continue;
            }
        }
        association.expression->accept(*this);
        if (association.isDefault()) {
            if (defaultIndex) {
                semanticError("duplicate default generic association", association.expression->getContext());
            } else {
                defaultIndex = i;
            }
            continue;
        }
        if (association.typeName->getType().sameQualifiedType(converted)) {
            matches.push_back(i);
        }
    }
    if (matches.size() > 1) {
        semanticError("generic selection has multiple matching associations", expression.getContext());
        return;
    }
    const std::optional<std::size_t> selected = matches.empty() ? defaultIndex
            : std::optional<std::size_t> { matches.front() };
    if (!selected) {
        semanticError("generic selection has no matching association", expression.getContext());
        return;
    }
    if (!associations[*selected].expression->hasResultSymbol(annotations())) {
        return;
    }
    expression.select(*selected, annotations());
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.getTypeSpecifier().resolveTypeof(*this);
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol(annotations()) || !expression.getTypeSpecifier().hasType()) {
        return;
    }

    type::Type target = expression.getTypeSpecifier().getType();
    if (target.isArray() || type::isBareFunction(target)) {
        semanticError("cast to array or function type ‘" + target.to_string() + "’", expression.getContext());
        return;
    }

    type::Type source = expression.operandType();
    if (type::isBareFunction(source)) {
        semanticError("cast of function designator is not supported", expression.getContext());
        return;
    }
    // Operand may be an array object or a dual-type multi-dim row (value already a pointer).
    // Codegen materializes AddressOf only when the value type is still an array.
    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(target));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    // Prefer Result symbol types (dual-type expressions may differ from expressionType()).
    const type::Type left = expression.leftOperandSymbol(annotations())->getType();
    const type::Type right = expression.rightOperandSymbol(annotations())->getType();
    rejectFunctionValue(left, expression.getContext());
    rejectFunctionValue(right, expression.getContext());

    const char op = expression.getOperator()->getLexeme().front();
    const type::PointerArithmeticInfo ptrArith = type::classifyPointerArithmetic(left, right, op);
    if (ptrArith.form != type::PointerArithmeticForm::None) {
        if (ptrArith.form == type::PointerArithmeticForm::Invalid) {
            semanticError("invalid operands to pointer arithmetic", expression.getContext());
            return;
        }
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(ptrArith.resultType));
        return;
    }

    typeCheck(left, right, expression.getContext());
    const type::Type resultType = applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());
    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

    if (expression.rightOperandType().isPrimitive() && !expression.rightOperandType().getPrimitive().isFloating()) {
        maybeSetConversion(expression.getRightOperand(),
                type::integerPromote(expression.rightOperandSymbol(annotations())->getType()),
                symbolTable, annotations());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(expression.leftOperandType()));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

    const type::Type left = expression.leftOperandSymbol(annotations())->getType();
    const type::Type right = expression.rightOperandSymbol(annotations())->getType();
    typeCheck(left, right, expression.getContext());
    applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());

    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setTruthyLabel(annotations(), symbolTable.newLabel());
    expression.setFalsyLabel(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    const type::Type left = expression.leftOperandSymbol(annotations())->getType();
    const type::Type right = expression.rightOperandSymbol(annotations())->getType();
    rejectFunctionValue(left, expression.getContext());
    rejectFunctionValue(right, expression.getContext());
    typeCheck(left, right, expression.getContext());
    const type::Type resultType = applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());
    expression.setType(resultType);
    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    expression.visitTrueExpression(*this);
    expression.visitFalseExpression(*this);

    if (!expression.getCondition()->hasResultSymbol(annotations())
            || !expression.getTrueExpression()->hasResultSymbol(annotations())
            || !expression.getFalseExpression()->hasResultSymbol(annotations())) {
        return;
    }

    rejectFunctionValue(expression.conditionSymbol(annotations())->getType(), expression.getContext());
    rejectFunctionValue(expression.trueSymbol(annotations())->getType(), expression.getContext());
    rejectFunctionValue(expression.falseSymbol(annotations())->getType(), expression.getContext());

    typeCheck(
            expression.trueSymbol(annotations())->getType(),
            expression.falseSymbol(annotations())->getType(),
            expression.getContext());

    // Result type follows the true arm after typeCheck (same policy as other binary ops).
    const type::Type resultType = expression.trueSymbol(annotations())->getType();
    expression.setType(resultType);
    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(resultType));
    expression.setFalsyLabel(annotations(), symbolTable.newLabel());
    expression.setExitLabel(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }

    // C assignment is not an lvalue; check the LHS operand (may clear parse-time folds).
    if (expression.getLeftOperand()->isLval()) {
        const type::Type left = expression.leftOperandType();
        auto* right = expression.getRightOperand();
        // Dual-type structure address is not a structure object rvalue.
        if (right->holdsAggregateAddress() && left.isRecord()) {
            semanticError("assignment of structure from dual-type aggregate is not supported",
                    expression.getContext());
            return;
        }
        rejectFunctionValue(left, expression.getContext());
        type::Type srcType = assignSourceType(*right, left, annotations());
        if (!right->holdsAggregateAddress()) {
            rejectFunctionValue(srcType, expression.getContext());
        }
        typeCheck(srcType, left, expression.getContext());
        maybeSetConversion(right,
                type::assignmentConvertTarget(expression.getOperator()->getLexeme(), left, srcType),
                symbolTable, annotations());

        expression.setTypeAndResult(annotations(), *expression.leftOperandSymbol(annotations()));
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    // Comma operator: value and type of the right operand
    expression.setType(expression.rightOperandType());
    expression.setResultSymbol(annotations(), *expression.rightOperandSymbol(annotations()));
}

void SemanticAnalysisVisitor::visit(ast::Operator&) {
}


} // namespace semantic_analyzer
