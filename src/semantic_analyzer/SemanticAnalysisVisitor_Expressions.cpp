#include "SemanticAnalysisVisitorInternal.h"
#include "AggregateInitSink.h"
#include "InitializerLowering.h"
#include "ast/LogicalExpression.h"
#include "symbols/AddressPlan.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::checkIncrementOperand(bool isLval, const type::Type& operandType,
        const translation_unit::Context& context) {
    if (!isLval) {
        semanticError("lvalue required as increment operand", context);
    }
    if (!type::isRealType(operandType) && !operandType.isPointer()) {
        semanticError("invalid operand to increment (real or pointer type required)", context);
    }
}

void SemanticAnalysisVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    // Operand failed to resolve (e.g. undeclared identifier) - error already reported.
    if (!arrayAccess.getLeftOperand()->hasResultSymbol(annotations())
            || !arrayAccess.getRightOperand()->hasResultSymbol(annotations())) {
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
    symbols::IndexPlan indexPlan;
    indexPlan.elementSize = sub.elementStride;
    indexPlan.elementType = sub.elementType;
    const type::Type baseExprType =
            baseOperand == symbols::BinaryOperand::Left ? leftExpr : rightExpr;
    indexPlan.baseMode = (sub.baseIsArray && !type::hasRuntimeSize(baseExprType))
            ? symbols::AddressBaseMode::LeaObject
            : symbols::AddressBaseMode::PointerValue;
    indexPlan.baseOperand = baseOperand;
    annotations().setAddressPlan(&arrayAccess, symbols::AddressPlan { indexPlan });

    // Array element: dual-type address. Record element: object Result so
    // assign/call/return copy the struct (not an address).
    if (sub.elementType.isArray()) {
        auto addrSym = symbolTable.createTemporarySymbol(sub.elementType.decayArray());
        annotations().setValue(&arrayAccess, symbols::ValueSlot::Lvalue, addrSym);
        arrayAccess.setAggregateAddressResult(annotations(), addrSym, sub.elementType);
    } else {
        annotations().setValue(&arrayAccess, symbols::ValueSlot::Lvalue,
                symbolTable.createTemporarySymbol(type::pointer(sub.elementType)));
        arrayAccess.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(sub.elementType));
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    const std::string& name = identifier.getIdentifier();
    if (name == "__func__" || name == "__FUNCTION__" || name == "__PRETTY_FUNCTION__") {
        if (currentFunctionName.empty()) {
            semanticError("__func__ used outside a function", identifier.getContext());
            return;
        }
        const std::string literal = "\"" + currentFunctionName + "\"";
        annotations().setString(&identifier, symbols::StringSlot::ConstantLabel, symbolTable.newConstant(literal));
        identifier.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(
                type::pointer(type::signedCharacter(), { type::Qualifier::CONST })));
        identifier.setAsRvalue();
        return;
    }

    if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        identifier.clearFoldedConstant();
        identifier.setTypeAndResult(annotations(), symbolTable.lookup(identifier.getIdentifier()));
    } else if (symbolTable.hasFunction(identifier.getIdentifier())) {
        // functions[] is the sole function namespace (not a dual ValueEntry table).
        // Designator result is already pointer-to-function; label on FunctionDesignatorPlan.
        auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
        type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
        type::Type ptrType = type::pointer(fnType);
        identifier.clearFoldedConstant();
        identifier.setFunctionDesignatorResult(annotations(),
                symbolTable.createTemporarySymbol(ptrType), fnType);
        symbols::FunctionDesignatorPlan plan;
        plan.functionName = functionEntry.getName();
        annotations().setAddressPlan(&identifier, symbols::AddressPlan { plan });
    } else if (symbolTable.hasEnumConstant(identifier.getIdentifier())) {
        type::IntegerConstant ice = symbolTable.getEnumConstant(identifier.getIdentifier());
        identifier.setFoldedConstant(ice);
        identifier.setTypeAndResult(annotations(),
                symbolTable.createTemporarySymbol(ice.type));
    } else {
        semanticError("symbol `" + identifier.getIdentifier() + "` is not defined", identifier.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(constant.expressionType()));
}

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    auto address = symbolTable.createTemporarySymbol(type::pointer(type::signedCharacter()));
    stringLiteral.setAggregateAddressResult(annotations(), address, stringLiteral.expressionType());
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResultSymbol(annotations())) {
        return;
    }

    auto operandSymbol = *expression.getOperandExpression()->getResultSymbol(annotations());
    expression.setTypeAndResult(annotations(),
            symbolTable.createTemporarySymbol(operandSymbol.getType()));

    checkIncrementOperand(expression.isLval(),
            expression.getOperandExpression()->valueType(annotations()), expression.getContext());
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResultSymbol(annotations())) {
        return;
    }

    expression.setTypeAndResult(annotations(),
            *expression.getOperandExpression()->getResultSymbol(annotations()));

    checkIncrementOperand(expression.isLval(),
            expression.getOperandExpression()->valueType(annotations()), expression.getContext());
}

void SemanticAnalysisVisitor::visit(ast::TypeNameExpression& expression) {
    auto resolved = resolveTypeName(expression.getTypeName(), expression.getContext());
    if (!resolved) {
        return;
    }
    expression.setType(*resolved);
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (op == OperatorKind::Sizeof) {
        expression.visitOperand(*this);
        if (auto err = applySizeof(expression, gnuExtensions_, annotations(), symbolTable)) {
            semanticError(*err, expression.getContext());
        }
        return;
    }

    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResultSymbol(annotations())) {
        return;
    }

    switch (op) {
    case OperatorKind::AddressOf: {
        auto* operand = expression.getOperandExpression();
        // Offsetof fold extracted to SizeofOffsetof (FieldPlan + null arrow base).
        if (auto* member = dynamic_cast<ast::MemberAccess*>(operand)) {
            if (!annotations().addressPlan(member)) {
                break; // member visit failed; error already reported
            }
            if (tryApplyOffsetofFold(expression, operand, annotations(), symbolTable)) {
                break;
            }
        }
        // &function_designator is the function address (already pointer-to-function after
        // designator materialization). Do not add another pointer level (master #176 path).
        if (operand->holdsFunctionDesignator()) {
            expression.takeValueFrom(*operand, annotations());
            break;
        }
        if (symbols::bitFieldOf(annotations().addressPlan(operand))) {
            semanticError("cannot take address of bit-field", expression.getContext());
            expression.setTypeAndResult(annotations(),
                    symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
            break;
        }
        // ArrayAccess / MemberAccess already store Index/Field plans in their visits.
        // Only fill a plan when the operand has none yet.
        if (!annotations().addressPlan(operand)) {
            if (operand->getLvalueSymbol(annotations())) {
                annotations().setAddressPlan(operand, symbols::AddressPlan { symbols::LvaluePlan {} });
            } else {
                annotations().setAddressPlan(operand, symbols::AddressPlan { symbols::ResultAddressOfPlan {} });
            }
        }
        markAddressOnly(*operand, annotations());
        expression.setTypeAndResult(annotations(),
                symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
        break;
    }
    case OperatorKind::Deref: {
        // Array operands decay to pointer-to-element (C 6.3.2.1), so *arr is arr[0].
        ast::Expression* operand = expression.getOperandExpression();
        type::Type rawOperandType = valueTypeAfterDesignatorDecay(*operand, annotations());
        type::Type operandType = rawOperandType.isArray() ? rawOperandType.decayArray() : rawOperandType;
        if (operandType.isPointer()) {
            type::Type pointee = operandType.dereference();
            if (pointee.isFunction()) {
                // *fp is a function designator (GNU sizeof is 1). Keep the pointer value.
                expression.setFunctionDesignatorResult(annotations(),
                        *expression.getOperandExpression()->getResultSymbol(annotations()), pointee);
                symbols::FunctionDesignatorPlan plan;
                if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(
                        annotations().addressPlan(expression.getOperandExpression()))) {
                    plan.functionName = d->functionName;
                }
                annotations().setAddressPlan(&expression, symbols::AddressPlan { plan });
            } else if (pointee.isArray()) {
                // *p for T(*)[N] is the array lvalue at that address (no load).
                auto addr = *expression.getOperandExpression()->getResultSymbol(annotations());
                annotations().setValue(&expression, symbols::ValueSlot::Lvalue, addr);
                expression.setAggregateAddressResult(annotations(), addr, pointee);
            } else if (rawOperandType.isArray()) {
                // *arr is arr[0]: distinct pointer lvalue; codegen leas then loads.
                annotations().setValue(&expression, symbols::ValueSlot::Lvalue,
                        symbolTable.createTemporarySymbol(operandType));
                expression.setTypeAndResult(annotations(),
                        symbolTable.createTemporarySymbol(pointee));
            } else {
                auto addr = *operand->getResultSymbol(annotations());
                annotations().setValue(&expression, symbols::ValueSlot::Lvalue, addr);
                expression.setTypeAndResult(annotations(),
                        symbolTable.createTemporarySymbol(pointee));
            }
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().to_string(),
                    expression.getContext());
        }
        break;
    }
    case OperatorKind::Add:
    case OperatorKind::Sub:
    case OperatorKind::BitNot: {
        const type::Type promoted = applyIntegerPromotion(
                *expression.getOperandExpression(), symbolTable, annotations());
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(promoted));
        break;
    }
    case OperatorKind::LogicalNot:
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
        annotations().setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
        annotations().setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
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
        if (association.isDefault()) {
            if (seenDefault) {
                semanticError("duplicate default generic association", association.expression->getContext());
            }
            seenDefault = true;
            arms[i] = { true, nullptr };
            association.expression->accept(*this);
            continue;
        }
        auto assocType = resolveTypeName(*association.typeName, expression.getContext());
        if (!assocType) {
            arms[i] = { false, nullptr };
            association.expression->accept(*this);
            continue;
        }
        resolved[i] = *assocType;
        arms[i] = { false, &resolved[i] };
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
    if (!associations[*choice.index].expression->hasResultSymbol(annotations())) {
        return;
    }
    expression.select(*choice.index, annotations());
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResultSymbol(annotations())) {
        return;
    }

    auto targetOpt = resolveTypeName(expression.getTypeName(), expression.getContext());
    if (!targetOpt) {
        return;
    }
    type::Type target = *targetOpt;
    if (target.isArray() || (target.isFunction() && !target.isPointer())) {
        semanticError("cast to array or function type ‘" + target.to_string() + "’", expression.getContext());
        return;
    }

    // Operand may be an array object or a dual-type multi-dim row (value already a pointer).
    // Codegen materializes AddressOf only when the value type is still an array.
    // Keep dual ownership: expression type and Result both reflect the cast target.
    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(target));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    // +/- need array-to-pointer decay (pointer arithmetic); other ops keep arrays as values only when needed.
    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        analyzeAsRvalue(expression.getLeftOperand());
        analyzeAsRvalue(expression.getRightOperand());
    } else {
        expression.visitLeftOperand(*this);
        expression.visitRightOperand(*this);
    }

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }

    // Pointer +/- integer: scale the integer by pointee size (C 6.5.6).
    // Match pointer-index stride so p+1 agrees with p[1] (System V natural).
    // Use value types after decay (member arrays: expressionType T[N], valueType pointer).
    type::Type resultType = expression.getLeftOperand()->valueType(annotations());
    bool pointerArithmetic = false;
    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        const type::Type lt = expression.getLeftOperand()->valueType(annotations());
        const type::Type rt = expression.getRightOperand()->valueType(annotations());
        const char arithOp = (op == OperatorKind::Add) ? '+' : '-';
        type::PointerArithmeticInfo info = type::classifyPointerArithmetic(lt, rt, arithOp);
        if (info.form == type::PointerArithmeticForm::Invalid) {
            semanticError("invalid operands to pointer arithmetic", expression.getContext());
            return;
        }
        if (info.form != type::PointerArithmeticForm::None) {
            resultType = info.resultType;
            pointerArithmetic = true;
            auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
            if (info.form == type::PointerArithmeticForm::PtrMinusPtr) {
                annotations().setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerDifferencePlan { info.strideBytes, scaleTemp.getName() } });
            } else {
                const bool scaleRight = info.form != type::PointerArithmeticForm::IntPlusPtr;
                annotations().setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerScalePlan { info.strideBytes, scaleTemp.getName(), scaleRight } });
            }
        }
    }

    if (!pointerArithmetic) {
        if (!checkArithmeticCompatible(
                expression.getLeftOperand()->valueType(annotations()),
                expression.getRightOperand()->valueType(annotations()),
                expression.getContext())) {
            return;
        }
        resultType = applyUsualArithmeticConversions(
                *expression.getLeftOperand(), *expression.getRightOperand(),
                symbolTable, annotations());
        if (op == OperatorKind::Mod && type::isComplex(resultType)) {
            semanticError("invalid operands to % (complex type)", expression.getContext());
            return;
        }
    }

    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }

    if (type::isIntegral(expression.getLeftOperand()->valueType(annotations()))
            && type::isIntegral(expression.getRightOperand()->valueType(annotations()))) {
        const type::Type resultType = applyIntegerPromotion(
                *expression.getLeftOperand(), symbolTable, annotations());
        applyIntegerPromotion(*expression.getRightOperand(), symbolTable, annotations());
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(resultType));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    // Value operands: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getLeftOperand());
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }

    const type::Type left = type::afterLvalueConversion(
            expression.getLeftOperand()->valueType(annotations()));
    const type::Type right = type::afterLvalueConversion(
            expression.getRightOperand()->valueType(annotations()));
    const bool pointerCompare = (left.isPointer() && right.isPointer())
            || (left.isPointer() && type::isIntegral(right))
            || (right.isPointer() && type::isIntegral(left));
    if (!pointerCompare) {
        if (!checkValueCompatible(
                expression.getLeftOperand()->valueType(annotations()),
                expression.getRightOperand()->valueType(annotations()),
                expression.getContext())) {
            return;
        }
        const type::Type uac = applyUsualArithmeticConversions(
                *expression.getLeftOperand(), *expression.getRightOperand(),
                symbolTable, annotations());
        const ast::OperatorKind cmp = expression.getOperator()->getKind();
        if (type::isComplex(uac) && cmp != ast::OperatorKind::Eq && cmp != ast::OperatorKind::Ne) {
            semanticError("invalid operands to relational operator (complex type)", expression.getContext());
            return;
        }
    }

    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    annotations().setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
    annotations().setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }
    if (!checkValueCompatible(
            expression.getLeftOperand()->valueType(annotations()),
            expression.getRightOperand()->valueType(annotations()),
            expression.getContext())) {
        return;
    }
    if (!type::isIntegral(expression.getLeftOperand()->valueType(annotations()))
            || !type::isIntegral(expression.getRightOperand()->valueType(annotations()))) {
        semanticError("invalid operands to binary bitwise operator", expression.getContext());
        return;
    }

    const type::Type resultType = applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());
    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::analyzeLogical(ast::LogicalExpression& expression) {
    analyzeAsRvalue(expression.getLeftOperand());
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }
    const type::Type left = type::afterLvalueConversion(
            expression.getLeftOperand()->valueType(annotations()));
    const type::Type right = type::afterLvalueConversion(
            expression.getRightOperand()->valueType(annotations()));
    if (!type::isProductScalar(left) || !type::isProductScalar(right)) {
        semanticError("invalid operands to logical operator (scalar required)", expression.getContext());
        return;
    }

    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    annotations().setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    analyzeLogical(expression);
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    analyzeLogical(expression);
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    // Value arms: visit + array-to-pointer decay (C 6.3.2.1 / 6.5.15).
    analyzeAsRvalue(expression.getTrueExpression());
    analyzeAsRvalue(expression.getFalseExpression());

    if (!expression.getCondition()->hasResultSymbol(annotations())
            || !expression.getTrueExpression()->hasResultSymbol(annotations())
            || !expression.getFalseExpression()->hasResultSymbol(annotations())) {
        return;
    }

    // Value types after decay (member arrays keep expressionType as T[N] for sizeof).
    const type::Type trueTy = expression.getTrueExpression()->valueType(annotations());
    const type::Type falseTy = expression.getFalseExpression()->valueType(annotations());
    type::Type resultType = type::voidType();
    if (trueTy.isVoid() || falseTy.isVoid()) {
        resultType = type::voidType();
    } else {
        const std::optional<type::Type> result = type::conditionalResultType(trueTy, falseTy);
        if (!result) {
            semanticError("incompatible operand types in conditional expression", expression.getContext());
            return;
        }
        resultType = *result;
        maybeSetConversion(expression.getTrueExpression(), resultType, symbolTable, annotations());
        maybeSetConversion(expression.getFalseExpression(), resultType, symbolTable, annotations());
    }
    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(resultType));
    annotations().setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
    annotations().setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
    annotations().setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this); // lvalue: no decay
    // RHS value context: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }

    // C assignment is not an lvalue; check the LHS after visiting it so
    // _Generic select() can adopt the selected arm's value category.
    if (expression.getLeftOperand()->isLval()) {
        if (!checkProductAssign(
                expression.getLeftOperand()->valueType(annotations()),
                expression.getRightOperand()->valueType(annotations()),
                expression.getContext(),
                expression.getRightOperand())) {
            return;
        }

        const type::Type dest = expression.getLeftOperand()->valueType(annotations());
        const type::Type src = expression.getRightOperand()->valueType(annotations());
        maybeSetConversion(expression.getRightOperand(),
                type::assignmentConvertTarget(expression.getOperator()->getLexeme(), dest, src),
                symbolTable, annotations());

        // Pointer += / -= integer: scale like p = p + n (C 6.5.16.2 / 6.5.6).
        // git xdiff: changed += 1 then free(changed - 1) requires matching strides.
        using ast::OperatorKind;
        const OperatorKind op = expression.getOperator()->getKind();
        if (op == OperatorKind::AddAssign || op == OperatorKind::SubAssign) {
            const char arithOp = (op == OperatorKind::AddAssign) ? '+' : '-';
            type::PointerArithmeticInfo info = type::classifyPointerArithmetic(
                    expression.getLeftOperand()->valueType(annotations()),
                    expression.getRightOperand()->valueType(annotations()),
                    arithOp);
            if (info.form == type::PointerArithmeticForm::PtrPlusInt
                    || info.form == type::PointerArithmeticForm::PtrMinusInt) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                annotations().setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerScalePlan { info.strideBytes, scaleTemp.getName(), true } });
            }
        }

        expression.setTypeAndResult(annotations(),
                *expression.getLeftOperand()->getResultSymbol(annotations()));
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResultSymbol(annotations())
            || !expression.getRightOperand()->hasResultSymbol(annotations())) {
        return;
    }
    // Comma operator: value and type of the right operand
    expression.setTypeAndResult(annotations(),
            *expression.getRightOperand()->getResultSymbol(annotations()));
}

void SemanticAnalysisVisitor::visit(ast::Operator&) {
}

void SemanticAnalysisVisitor::visit(ast::InitializerListExpression& expression) {
    // Brace lists are not value expressions; target-type policy lives on the
    // declarator / compound-literal (peel scalar, or lowerToFieldInits).
    expression.visitElements(*this);
}

void SemanticAnalysisVisitor::visit(ast::CompoundLiteralExpression& expression) {
    if (expression.getInitializer()) {
        expression.getInitializer()->accept(*this);
    }

    const translation_unit::Context litCtx = expression.getContext();
    auto objectTypeOpt = resolveTypeName(expression.getTypeName(), litCtx);
    if (!objectTypeOpt) {
        return;
    }
    type::Type objectType = *objectTypeOpt;
    auto* initExpr = expression.getInitializer();
    // String-to-char[] is sink placeStringArray only (no AST brace rewrite).
    if (auto err = completeIncompleteArrayFromInitializer(objectType, initExpr)) {
        semanticError(std::move(*err), litCtx);
        return;
    }
    if (type::isIncompleteObjectType(objectType)) {
        semanticError("compound literal has incomplete type ‘" + objectType.to_string() + "’",
                litCtx);
        return;
    }

    symbols::ValueEntry home = symbolTable.isAtFileScope()
            ? symbolTable.createUnnamedStaticObject(objectType, litCtx)
            : symbolTable.createTemporarySymbol(objectType);
    annotations().setValue(&expression, symbols::ValueSlot::Object, home);
    expression.setTypeAndResult(annotations(), home);

    if (home.isGlobal()) {
        AggregateInitHost host {
            annotations(),
            [this](std::string msg, const translation_unit::Context& ctx) {
                semanticError(std::move(msg), ctx);
            }
        };
        if (initExpr) {
            type::Type completed = objectType;
            std::vector<symbols::DataWord> words;
            const DataWordsLowering lowered =
                    lowerToDataWords(objectType, initExpr, host, completed, words);
            if (lowered == DataWordsLowering::Ok) {
                symbolTable.setGlobalInitializer(home.getName(),
                        symbols::MultiWordInit { std::move(words) });
                return;
            }
            if (lowered == DataWordsLowering::Failed) {
                return;
            }
            symbols::GlobalInitializer init;
            if (tryFoldGlobalInit(initExpr, objectType, annotations(), init)) {
                symbolTable.setGlobalInitializer(home.getName(), std::move(init));
                return;
            }
            semanticError("global initializer is not a constant expression", litCtx);
        }
        return;
    }

    if (initExpr) {
        AggregateInitHost host {
            annotations(),
            [this](std::string msg, const translation_unit::Context& ctx) {
                semanticError(std::move(msg), ctx);
            }
        };
        for (auto& init : lowerToFieldInits(objectType, initExpr, symbolTable, host)) {
            annotations().addFieldInit(&expression, std::move(init));
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::MemberAccess& expression) {
    expression.getBase()->accept(*this);
    if (!expression.getBase()->hasResultSymbol(annotations()) || !expression.getBase()->hasExpressionType()) {
        return;
    }
    const bool isArrow = expression.isArrow();
    const auto record = type::memberAccessRecordType(expression.getBase()->expressionType(), isArrow);
    if (!record) {
        semanticError(isArrow ? "base of '->' is not a pointer to structure or union"
                              : "request for member in non-structure or non-union type",
                expression.getContext());
        return;
    }
    auto found = type::lookupMember(*record, expression.getMemberName());
    if (!found) {
        semanticError("no member named `" + expression.getMemberName() + "` in structure or union",
                expression.getContext());
        return;
    }
    type::Type memberTy = found->type;
    symbols::FieldPlan fieldPlan;
    fieldPlan.fieldOffsetBytes = found->offsetBytes;
    fieldPlan.bitField = found->bitField;
    annotations().setAddressPlan(&expression, symbols::AddressPlan { fieldPlan });
    // Array members: field address is pointer-to-element (&arr[0]), not
    // pointer-to-array. Decay sites reuse this lvalue; pointer-to-array would
    // make s.buf + i scale by sizeof(buf) (git sha1dc: ctx->buffer + left).
    if (memberTy.isArray()) {
        auto addrSym = symbolTable.createTemporarySymbol(memberTy.decayArray());
        annotations().setValue(&expression, symbols::ValueSlot::Lvalue, addrSym);
        expression.setAggregateAddressResult(annotations(), addrSym, memberTy);
    } else {
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(memberTy));
        annotations().setValue(&expression, symbols::ValueSlot::Lvalue,
                symbolTable.createTemporarySymbol(type::pointer(memberTy)));
    }
}

} // namespace semantic_analyzer
