#include "SemanticAnalysisVisitorInternal.h"
#include "types/TypeQuery.h"

#include "ast/CompoundLiteral.h"
#include "ast/InitializerListExpression.h"

namespace semantic_analyzer {

namespace {

void checkIncrementOperand(SemanticAnalysisVisitor& visitor, bool isLval,
        const type::Type& operandType, const translation_unit::Context& context) {
    if (!isLval) {
        visitor.semanticError("lvalue required as increment operand", context);
    }
    if (!type::isRealType(operandType) && !operandType.isPointer()) {
        visitor.semanticError("invalid operand to increment (real or pointer type required)", context);
    }
}

// C: && / || require scalar operands; arms need not be assignment-compatible.
void checkLogicalScalarOperands(SemanticAnalysisVisitor& visitor, const type::Type& leftRaw,
        const type::Type& rightRaw, const translation_unit::Context& context) {
    const type::Type left = type::afterLvalueConversion(leftRaw);
    const type::Type right = type::afterLvalueConversion(rightRaw);
    if (type::isProductScalar(left) && type::isProductScalar(right)) {
        return;
    }
    if (type::isBareFunction(leftRaw)) {
        visitor.semanticError("function designator used as a value is not supported", context);
    }
    if (type::isBareFunction(rightRaw)) {
        visitor.semanticError("function designator used as a value is not supported", context);
    }
    visitor.semanticError("invalid operands to logical operator (scalar required)", context);
}

} // namespace

void SemanticAnalysisVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    if (!arrayAccess.hasLeftOperandSymbol(annotations()) || !arrayAccess.hasRightOperandSymbol(annotations())) {
        return;
    }

    ast::Expression* left = arrayAccess.getLeftOperand();
    ast::Expression* right = arrayAccess.getRightOperand();
    const type::Type leftExpr = left->expressionType();
    const type::Type leftValue = left->valueType(annotations());
    const type::Type rightExpr = right->expressionType();
    const type::Type rightValue = right->valueType(annotations());

    symbols::BinaryOperand baseOperand = symbols::BinaryOperand::Left;
    type::ArraySubscriptInfo sub;
    if (type::isSubscriptBase(leftExpr, leftValue)) {
        sub = type::arraySubscriptInfo(leftExpr, leftValue);
    } else if ((type::isIntegralScalar(leftExpr) || type::isIntegralScalar(leftValue))
            && type::isSubscriptBase(rightExpr, rightValue)) {
        baseOperand = symbols::BinaryOperand::Right;
        sub = type::arraySubscriptInfo(rightExpr, rightValue);
    }
    if (!sub.valid()) {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
        return;
    }

    type::Type elementType = sub.elementType;

    symbols::IndexPlan indexPlan;
    indexPlan.elementSize = sub.elementStride;
    indexPlan.baseMode = sub.baseIsArray ? symbols::AddressBaseMode::LeaObject
                                         : symbols::AddressBaseMode::PointerValue;
    indexPlan.baseOperand = baseOperand;

    if (elementType.isArray()) {
        auto addr = symbolTable.createTemporarySymbol(type::pointer(elementType.getElementType()));
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
    const bool isArrow = memberAccess.isArrow();
    const auto record = type::memberAccessRecordType(memberAccess.getBase()->getType(), isArrow);
    if (!record) {
        semanticError(isArrow ? "base of '->' is not a pointer to structure or union"
                              : "request for member in non-structure or non-union type",
                memberAccess.getContext());
        return;
    }

    auto found = type::lookupMember(*record, memberAccess.getMemberName());
    if (!found) {
        semanticError("no member named ‘" + memberAccess.getMemberName() + "’ in structure or union",
                memberAccess.getContext());
        return;
    }
    const type::Type addrType = found->type.isArray()
            ? type::pointer(found->type.getElementType())
            : type::pointer(found->type);
    auto fieldAddr = symbolTable.createTemporarySymbol(addrType);
    memberAccess.setLvalueSymbol(annotations(), fieldAddr);
    symbols::FieldPlan fieldPlan;
    fieldPlan.fieldOffsetBytes = found->offsetBytes;
    fieldPlan.bitField = found->bitField;
    annotations().setAddressPlan(&memberAccess, symbols::AddressPlan { fieldPlan });
    if (found->type.isArray()) {
        memberAccess.setAggregateAddressResult(annotations(), fieldAddr, found->type);
    } else {
        memberAccess.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(found->type));
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(constant.getType()));
}

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    auto address = symbolTable.createTemporarySymbol(type::pointer(type::signedCharacter()));
    stringLiteral.setAggregateAddressResult(annotations(), address, stringLiteral.expressionType());
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

    checkIncrementOperand(*this, expression.isLval(), expression.operandType(), expression.getContext());
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol(annotations())) {
        return;
    }
    rejectFunctionValue(expression.operandType(), expression.getContext());

    expression.setType(expression.operandType());
    expression.setResultSymbol(annotations(), *expression.operandSymbol(annotations()));

    checkIncrementOperand(*this, expression.isLval(), expression.operandType(), expression.getContext());
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    const auto& lexeme = expression.getOperator()->getLexeme();
    if (lexeme == "sizeof") {
        expression.visitOperand(*this);
        if (!expression.hasOperandSymbol(annotations())) {
            expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
            return;
        }
        if (symbols::bitFieldOf(annotations().addressPlan(expression.getOperandExpression()))) {
            semanticError("invalid application of sizeof to a bit-field", expression.getContext());
            expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
            return;
        }
        const type::Type measured = expression.operandType();
        if (auto bytes = type::sizeofObject(measured, gnuExtensions_)) {
            expression.setSizeofValue(*bytes);
        } else {
            semanticError(
                    "invalid application of ‘sizeof’ to incomplete type ‘" + measured.to_string() + "’",
                    expression.getContext());
        }
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
            if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(
                    annotations().addressPlan(expression.getOperandExpression()))) {
                annotations().setAddressPlan(&expression, symbols::AddressPlan { *d });
            }
            break;
        }
        if (symbols::bitFieldOf(annotations().addressPlan(expression.getOperandExpression()))) {
            semanticError("cannot take address of bit-field", expression.getContext());
            expression.setResultSymbol(annotations(),
                    symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
            break;
        }
        rejectFunctionValue(expression.operandType(), expression.getContext());
        expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
        break;
    }
    case '*': {
        const type::Type valueType = expression.operandSymbol(annotations())->getType();
        rejectFunctionValue(valueType, expression.getContext());
        type::Type operandType = expression.operandType();
        // Value already a pointer (e.g. multi-dim a[i] decayed row, or int(*)[N]).
        if (valueType.isPointer()) {
            type::Type pointee = valueType.dereference();
            if (type::isBareFunction(pointee)) {
                expression.setFunctionDesignatorResult(annotations(),
                        *expression.operandSymbol(annotations()), pointee);
                symbols::FunctionDesignatorPlan plan;
                if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(
                        annotations().addressPlan(expression.getOperandExpression()))) {
                    plan.functionName = d->functionName;
                }
                annotations().setAddressPlan(&expression, symbols::AddressPlan { plan });
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
    case '-':
    case '~': {
        rejectFunctionValue(expression.operandType(), expression.getContext());
        const type::Type promoted = applyIntegerPromotion(
                *expression.getOperandExpression(), symbolTable, annotations());
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(promoted));
        break;
    }
    case '!':
        rejectFunctionValue(type::afterLvalueConversion(expression.operandType()), expression.getContext());
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

    auto& associations = expression.associations();
    std::vector<type::Type> resolved(associations.size(), type::voidType());
    std::vector<type::GenericArmView> arms(associations.size());
    bool seenDefault = false;
    for (std::size_t i = 0; i < associations.size(); ++i) {
        auto& association = associations[i];
        if (association.typeName) {
            association.typeName->resolveTypeof(*this);
            if (!association.typeName->hasType()) {
                semanticError("cannot determine type of generic association", expression.getContext());
                arms[i] = { false, nullptr };
            } else {
                resolved[i] = association.typeName->getType();
                arms[i] = { false, &resolved[i] };
            }
        } else {
            if (seenDefault) {
                semanticError("duplicate default generic association", association.expression->getContext());
            }
            seenDefault = true;
            arms[i] = { true, nullptr };
        }
        association.expression->accept(*this);
    }
    const type::GenericSelectionChoice choice = type::selectGenericAssociation(converted, arms);
    if (choice.status == type::GenericSelectionStatus::MultipleMatches) {
        semanticError("generic selection has multiple matching associations", expression.getContext());
        return;
    }
    if (choice.status != type::GenericSelectionStatus::Ok || !choice.index) {
        semanticError("generic selection has no matching association", expression.getContext());
        return;
    }
    const std::optional<std::size_t> selected = choice.index;
    if (!associations[*selected].expression->hasResultSymbol(annotations())) {
        return;
    }
    expression.select(*selected, annotations());
}

void SemanticAnalysisVisitor::visit(ast::CompoundLiteral& expression) {
    expression.getTypeSpecifier().resolveTypeof(*this);
    expression.initializer().accept(*this);
    if (!expression.getTypeSpecifier().hasType()) {
        return;
    }

    type::Type target = expression.getTypeSpecifier().getType();
    ast::InitializerListExpression& list = expression.initializer();
    if (!applyIncompleteArrayBound(target, &list, expression.getContext())) {
        return;
    }
    if (type::isIncompleteObjectType(target)) {
        semanticError("compound literal has incomplete type ‘" + target.to_string() + "’",
                expression.getContext());
        return;
    }
    ValueEntry home = symbolTable.isAtFileScope()
            ? symbolTable.createUnnamedStaticObject(target, expression.getContext())
            : symbolTable.createTemporarySymbol(target);
    expression.setResultSymbol(annotations(), home);
    if (home.isGlobal()) {
        lowerStaticInit(home.getName(), target, &list, expression.getContext());
        return;
    }
    if (target.isRecord() || target.isArray()) {
        planLocalAggregateFieldInits(&expression, target, &list, expression.getContext());
    } else {
        lowerLocalScalarBraceList(list, target, expression.getContext());
    }
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

    decayArrayValue(*expression.getLeftOperand(), symbolTable, annotations());
    decayArrayValue(*expression.getRightOperand(), symbolTable, annotations());
    const type::Type leftValue = expression.leftOperandSymbol(annotations())->getType();
    const type::Type rightValue = expression.rightOperandSymbol(annotations())->getType();

    const char op = expression.getOperator()->getLexeme().front();
    const type::PointerArithmeticInfo ptrArith =
            type::classifyPointerArithmetic(leftValue, rightValue, op);
    if (ptrArith.form == type::PointerArithmeticForm::Invalid) {
        semanticError("invalid operands to pointer arithmetic", expression.getContext());
        return;
    }
    if (ptrArith.form != type::PointerArithmeticForm::None) {
        expression.setResultSymbol(annotations(),
                symbolTable.createTemporarySymbol(ptrArith.resultType));
        return;
    }
    if (!type::productArithmeticCompatible(leftValue, rightValue)) {
        semanticError("invalid operands to binary operator", expression.getContext());
        return;
    }

    const type::Type resultType = applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());
    if (op == '%' && type::isComplex(resultType)) {
        semanticError("invalid operands to % (complex type)", expression.getContext());
        return;
    }
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

    if (type::isIntegral(expression.leftOperandType())
            && type::isIntegral(expression.rightOperandType())) {
        const type::Type resultType = applyIntegerPromotion(
                *expression.getLeftOperand(), symbolTable, annotations());
        applyIntegerPromotion(*expression.getRightOperand(), symbolTable, annotations());
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(resultType));
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
    // Decay array/function designators before the gate (C value context).
    decayArrayValue(*expression.getLeftOperand(), symbolTable, annotations());
    decayArrayValue(*expression.getRightOperand(), symbolTable, annotations());
    const type::Type leftRaw = expression.leftOperandSymbol(annotations())->getType();
    const type::Type rightRaw = expression.rightOperandSymbol(annotations())->getType();
    const type::Type left = type::afterLvalueConversion(leftRaw);
    const type::Type right = type::afterLvalueConversion(rightRaw);

    // Pointer equality/relational: two pointers, or pointer vs integer 0 (null).
    const bool leftPtr = left.isPointer();
    const bool rightPtr = right.isPointer();
    const bool pointerCompare = (leftPtr && rightPtr)
            || (leftPtr && type::isIntegral(right))
            || (rightPtr && type::isIntegral(left));
    if (!pointerCompare) {
        rejectFunctionValue(leftRaw, expression.getContext());
        rejectFunctionValue(rightRaw, expression.getContext());
        checkOperandTypes(leftRaw, rightRaw, expression.getContext());
        const type::Type uac = applyUsualArithmeticConversions(
                *expression.getLeftOperand(), *expression.getRightOperand(),
                symbolTable, annotations());
        const std::string& op = expression.getOperator()->getLexeme();
        if (type::isComplex(uac) && op != "==" && op != "!=") {
            semanticError("invalid operands to relational operator (complex type)", expression.getContext());
            return;
        }
    }

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
    checkOperandTypes(left, right, expression.getContext());
    const type::Type resultType = applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());
    if (type::isComplex(resultType)) {
        semanticError("invalid operands to bitwise operator (complex type)", expression.getContext());
        return;
    }
    expression.setType(resultType);
    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol(annotations()) || !expression.hasRightOperandSymbol(annotations())) {
        return;
    }
    checkLogicalScalarOperands(*this, expression.leftOperandType(), expression.rightOperandType(),
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
    checkLogicalScalarOperands(*this, expression.leftOperandType(), expression.rightOperandType(),
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

    auto* trueExpr = expression.getTrueExpression();
    auto* falseExpr = expression.getFalseExpression();
    const type::Type trueType = expression.trueSymbol(annotations())->getType();
    const type::Type falseType = expression.falseSymbol(annotations())->getType();
    const std::optional<type::Type> result = type::conditionalResultType(trueType, falseType);
    if (!result) {
        semanticError("incompatible operand types in conditional expression", expression.getContext());
        return;
    }
    decayArrayValue(*trueExpr, symbolTable, annotations());
    decayArrayValue(*falseExpr, symbolTable, annotations());

    expression.setType(*result);
    expression.setResultSymbol(annotations(), symbolTable.createTemporarySymbol(*result));
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
        rejectFunctionValue(left, expression.getContext());
        type::Type srcType = assignSourceType(*right, left, annotations());
        if (!right->holdsAggregateAddress()) {
            rejectFunctionValue(srcType, expression.getContext());
        }
        checkAssign(left, srcType, expression.getContext(), right);
        decayArrayValue(*right, symbolTable, annotations());
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
