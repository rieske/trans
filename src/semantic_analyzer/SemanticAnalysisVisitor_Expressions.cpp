#include "SemanticAnalysisVisitorInternal.h"
#include "AggregateInitSink.h"
#include "InitializerLowering.h"

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

    // Operand failed to resolve (e.g. undeclared identifier) — error already reported.
    if (!arrayAccess.getLeftOperand()->hasResult(annotations()) || !arrayAccess.getRightOperand()->hasResult(annotations())) {
        return;
    }

    const type::ArraySubscriptInfo sub = type::arraySubscriptInfo(
            arrayAccess.leftOperandType(),
            arrayAccess.getLeftOperand()->valueType(annotations()));
    if (!sub.valid()) {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
        return;
    }
    symbols::IndexPlan indexPlan;
    indexPlan.elementSize = sub.elementStride;
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

    long enumValue = 0;
    if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        identifier.clearFoldedConstant();
        identifier.setTypeAndResult(annotations(), symbolTable.lookup(identifier.getIdentifier()));
    } else if (symbolTable.hasFunction(identifier.getIdentifier())) {
        // Function designator decays to pointer-to-function when used as a value.
        auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
        type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
        type::Type ptrType = type::pointer(fnType);
        identifier.clearFoldedConstant();
        identifier.setFunctionDesignatorResult(annotations(), symbolTable.createTemporarySymbol(ptrType),
                functionEntry.getName());
    } else if (symbolTable.hasEnumConstant(identifier.getIdentifier())) {
        enumValue = symbolTable.getEnumConstant(identifier.getIdentifier());
        identifier.setFoldedConstant(enumValue);
        identifier.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
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
    stringLiteral.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(stringLiteral.expressionType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(annotations())) {
        return;
    }

    auto operandSymbol = *expression.getOperandExpression()->result(annotations());
    expression.setTypeAndResult(annotations(), operandSymbol);

    auto preOperationSymbolName = operandSymbol.getName() + "_pre";
    symbolTable.insertSymbol(preOperationSymbolName, operandSymbol.getType(), operandSymbol.getContext());
    annotations().setValue(&expression, symbols::ValueSlot::PreOperation, symbolTable.lookup(preOperationSymbolName));

    checkIncrementOperand(expression.isLval(),
            expression.getOperandExpression()->valueType(annotations()), expression.getContext());
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(annotations())) {
        return;
    }

    expression.setTypeAndResult(annotations(), *expression.getOperandExpression()->result(annotations()));

    checkIncrementOperand(expression.isLval(),
            expression.getOperandExpression()->valueType(annotations()), expression.getContext());
}

void SemanticAnalysisVisitor::visit(ast::TypeNameExpression& expression) {
    const translation_unit::Context ctx = expression.getContext();
    auto resolved = resolveTypeName(expression.getTypeName(), *this,
            [this](std::string msg, const translation_unit::Context& c) {
                semanticError(std::move(msg), c);
            },
            &ctx);
    if (!resolved) {
        return;
    }
    expression.setType(*resolved);
}

void SemanticAnalysisVisitor::visit(ast::OffsetofExpression& expression) {
    // Typed authority for offsetof: re-resolve TypeName and set folded integer
    // even when parse already seeded ICE via CSNB_Builtins.
    const translation_unit::Context ctx = expression.getContext();
    auto recordTypeOpt = resolveTypeName(expression.getTypeName(), *this,
            [this](std::string msg, const translation_unit::Context& c) {
                semanticError(std::move(msg), c);
            },
            &ctx);
    if (!recordTypeOpt) {
        return;
    }
    type::Type recordType = *recordTypeOpt;
    const type::OffsetofResult off = type::resolveOffsetof(recordType, expression.getMemberName());
    if (off.status != type::OffsetofStatus::Ok) {
        if (off.status == type::OffsetofStatus::BitField) {
            semanticError("cannot compute offset of bit-field `" + expression.getMemberName() + "`",
                    expression.getContext());
        } else if (off.status == type::OffsetofStatus::Incomplete) {
            semanticError("offsetof on incomplete type", expression.getContext());
        } else {
            semanticError("no member named `" + expression.getMemberName() + "` in offsetof",
                    expression.getContext());
        }
        return;
    }
    expression.setFoldedInteger(off.offsetBytes);
    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::unsignedLong()));
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (op == OperatorKind::Sizeof) {
        // Resolve operand type; do not rely on runtime evaluation of the operand.
        expression.visitOperand(*this);
        ast::Expression* operand = expression.getOperandExpression();
        if (operand && operand->hasExpressionType()
                && type::isIncompleteObjectType(operand->expressionType())) {
            semanticError("invalid application of sizeof to incomplete type '"
                    + operand->expressionType().to_string() + "'",
                    expression.getContext());
            expression.setTypeAndResult(annotations(),
                    symbolTable.createTemporarySymbol(type::unsignedLong()));
            return;
        }
        if (operand && symbols::bitFieldOf(annotations().addressPlan(operand))) {
            semanticError("invalid application of sizeof to a bit-field", expression.getContext());
            expression.setTypeAndResult(annotations(),
                    symbolTable.createTemporarySymbol(type::unsignedLong()));
            return;
        }
        applySizeofResult(expression, annotations(), symbolTable);
        return;
    }

    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(annotations())) {
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
        if (symbols::bitFieldOf(annotations().addressPlan(operand))) {
            semanticError("cannot take address of bit-field", expression.getContext());
            expression.setTypeAndResult(annotations(),
                    symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
            break;
        }
        // ArrayAccess / MemberAccess already store Index/Field plans in their visits.
        // Only fill a plan when the operand has none yet.
        if (!annotations().addressPlan(operand)) {
            if (operand->holdsFunctionDesignator()) {
                // FunctionDesignatorPlan already set by setFunctionDesignatorResult.
            } else if (operand->lvalueAnnotation(annotations())) {
                annotations().setAddressPlan(operand, symbols::AddressPlan { symbols::LvaluePlan {} });
            } else {
                annotations().setAddressPlan(operand, symbols::AddressPlan { symbols::ResultAddressOfPlan {} });
            }
        }
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
        break;
    }
    case OperatorKind::Deref: {
        // Array operands decay to pointer-to-element (C 6.3.2.1), so *arr is arr[0].
        type::Type operandType = expression.operandType();
        if (operandType.isArray()) {
            operandType = operandType.decayArray();
        }
        if (operandType.isPointer()) {
            type::Type pointee = operandType.dereference();
            if (pointee.isFunction()) {
                // *fp for a function pointer is a function designator; the call uses the
                // pointer value itself (no memory load). Keep the pointer as the result.
                expression.setTypeAndResult(annotations(), *expression.getOperandExpression()->result(annotations()));
            } else if (pointee.isArray()) {
                // *p for T(*)[N] is the array lvalue at that address (no load).
                auto addr = *expression.getOperandExpression()->result(annotations());
                annotations().setValue(&expression, symbols::ValueSlot::Lvalue, addr);
                expression.setAggregateAddressResult(annotations(), addr, pointee);
            } else {
                expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(pointee));
                annotations().setValue(&expression, symbols::ValueSlot::Lvalue, symbolTable.createTemporarySymbol(operandType));
            }
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().to_string(), expression.getContext());
        }
        break;
    }
    case OperatorKind::Add:
        expression.setTypeAndResult(annotations(), *expression.getOperandExpression()->result(annotations()));
        break;
    case OperatorKind::Sub:
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case OperatorKind::LogicalNot:
        expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
        annotations().setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
        annotations().setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
        break;
    case OperatorKind::BitNot:
        // Integer promotions apply to ~ (C 6.5.3.3).
        expression.setTypeAndResult(annotations(), 
                symbolTable.createTemporarySymbol(type::integerPromote(expression.operandType())));
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
            if (last->hasResult(annotations())) {
                expression.takeValueFrom(*last, annotations());
                return;
            }
        }
    }
    expression.setType(type::voidType());
}

void SemanticAnalysisVisitor::visit(ast::GenericSelection& expression) {
    expression.controllingExpression().accept(*this);
    if (!expression.controllingExpression().hasResult(annotations())
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
        if (association.isDefault()) {
            association.expression->accept(*this);
            if (defaultIndex) {
                semanticError("duplicate default generic association", association.expression->getContext());
            } else {
                defaultIndex = i;
            }
            continue;
        }
        const translation_unit::Context assocCtx = expression.getContext();
        auto assocType = resolveTypeName(*association.typeName, *this,
                [this](std::string msg, const translation_unit::Context& ctx) {
                    semanticError(std::move(msg), ctx);
                },
                &assocCtx);
        if (!assocType) {
            continue;
        }
        association.expression->accept(*this);
        if (assocType->sameQualifiedType(converted)) {
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
    if (!associations[*selected].expression->hasResult(annotations())) {
        return;
    }
    expression.select(*selected, annotations());
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(annotations())) {
        return;
    }

    const translation_unit::Context castCtx = expression.getContext();
    auto targetOpt = resolveTypeName(expression.getTypeName(), *this,
            [this](std::string msg, const translation_unit::Context& ctx) {
                semanticError(std::move(msg), ctx);
            },
            &castCtx);
    if (!targetOpt) {
        return;
    }
    type::Type target = *targetOpt;
    if (target.isArray() || (target.isFunction() && !target.isPointer())) {
        semanticError("cast to array or function type ‘" + target.to_string() + "’", expression.getContext());
        return;
    }

    type::Type source = expression.operandType();
    if (source.isFunction() && !source.isPointer()) {
        semanticError("cast of function designator is not supported", expression.getContext());
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

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
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
            if (info.strideBytes > 1) {
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
    }

    if (!pointerArithmetic) {
        requireArithmeticCompatible(
                expression.getLeftOperand()->valueType(annotations()),
                expression.getRightOperand()->valueType(annotations()),
                expression.getContext());
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

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }

    if (type::isIntegral(expression.getLeftOperand()->valueType(annotations()))
            && type::isIntegral(expression.getRightOperand()->valueType(annotations()))) {
        maybeSetConversion(expression.getRightOperand(),
                type::integerPromote(expression.getRightOperand()->valueType(annotations())),
                symbolTable, annotations());
        // Result type is the promoted left operand (C 6.5.7).
        expression.setTypeAndResult(annotations(), 
                symbolTable.createTemporarySymbol(type::integerPromote(expression.getLeftOperand()->valueType(annotations()))));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    // Value operands: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getLeftOperand());
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(annotations()),
            expression.getRightOperand()->valueType(annotations()),
            expression.getContext());
    const type::Type uac = applyUsualArithmeticConversions(
            *expression.getLeftOperand(), *expression.getRightOperand(),
            symbolTable, annotations());
    const ast::OperatorKind cmp = expression.getOperator()->getKind();
    if (type::isComplex(uac) && cmp != ast::OperatorKind::Eq && cmp != ast::OperatorKind::Ne) {
        semanticError("invalid operands to relational operator (complex type)", expression.getContext());
        return;
    }

    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    annotations().setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
    annotations().setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }
    requireValueCompatible(
            expression.getLeftOperand()->valueType(annotations()),
            expression.getRightOperand()->valueType(annotations()),
            expression.getContext());
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

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(annotations()),
            expression.getRightOperand()->valueType(annotations()),
            expression.getContext());

    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    annotations().setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(annotations()),
            expression.getRightOperand()->valueType(annotations()),
            expression.getContext());

    expression.setTypeAndResult(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));
    annotations().setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    // Value arms: visit + array-to-pointer decay (C 6.3.2.1 / 6.5.15).
    analyzeAsRvalue(expression.getTrueExpression());
    analyzeAsRvalue(expression.getFalseExpression());

    if (!expression.getCondition()->hasResult(annotations())
            || !expression.getTrueExpression()->hasResult(annotations())
            || !expression.getFalseExpression()->hasResult(annotations())) {
        return;
    }

    // Use value types after decay, not expressionType(). Member arrays keep
    // expressionType as T[N] for sizeof, but their result symbol is already
    // &member[0] (pointer). Using expressionType makes the ternary result an
    // array temporary; assignment/call-arg decay then takes &stack_temp
    // (git xdiff: buf = func_line ? func_line->buf : dummy - hunk funcname NUL).
    const type::Type trueTy = expression.getTrueExpression()->result(annotations())->getType();
    const type::Type falseTy = expression.getFalseExpression()->result(annotations())->getType();
    type::Type resultType = trueTy;
    if (!trueTy.isVoid() && !falseTy.isVoid()) {
        requireValueCompatible(trueTy, falseTy, expression.getContext());
    } else {
        // Void ternary used as a statement (assert macros: cond ? (void)0 : die()).
        resultType = type::voidType();
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

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }

    // C assignment is not an lvalue; check the LHS after visiting it so
    // _Generic select() can adopt the selected arm's value category.
    if (expression.getLeftOperand()->isLval()) {
        requireProductAssignable(
                expression.getLeftOperand()->valueType(annotations()),
                expression.getRightOperand()->valueType(annotations()),
                expression.getContext());

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
                if (info.strideBytes > 1) {
                    auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                    annotations().setPointerArithPlan(&expression, symbols::PointerArithPlan {
                            symbols::PointerScalePlan { info.strideBytes, scaleTemp.getName(), true } });
                }
            }
        }

        expression.setTypeAndResult(annotations(), *expression.getLeftOperand()->result(annotations()));
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(annotations()) || !expression.getRightOperand()->hasResult(annotations())) {
        return;
    }
    // Comma operator: value and type of the right operand
    expression.setTypeAndResult(annotations(), *expression.getRightOperand()->result(annotations()));
}

void SemanticAnalysisVisitor::visit(ast::Operator&) {
}

void SemanticAnalysisVisitor::visit(ast::InitializerListExpression& expression) {
    // Brace lists are not value expressions; target-type policy lives on the
    // declarator / compound-literal (peel scalar, or lowerToFieldInits).
    expression.visitElements(*this);
}

void SemanticAnalysisVisitor::visit(ast::CompoundLiteralExpression& expression) {
    // Materialize (type){ init } as a stack temporary; expand like local brace init.
    if (expression.getInitializer()) {
        expression.getInitializer()->accept(*this);
    }

    const translation_unit::Context litCtx = expression.getContext();
    auto objectTypeOpt = resolveTypeName(expression.getTypeName(), *this,
            [this](std::string msg, const translation_unit::Context& ctx) {
                semanticError(std::move(msg), ctx);
            },
            &litCtx);
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
    AggregateInitHost host {
        annotations(),
        [this](std::string msg, const translation_unit::Context& ctx) {
            semanticError(std::move(msg), ctx);
        }
    };
    type::Type completed = objectType;
    if (initExpr) {
        completed = lowerToFieldInits(objectType, initExpr, symbolTable, host,
                [&](symbols::StructFieldInit init) {
                    annotations().addStructFieldInit(&expression, std::move(init));
                });
    }
    auto object = symbolTable.createTemporarySymbol(completed);
    annotations().setValue(&expression, symbols::ValueSlot::Object, object);
    expression.setTypeAndResult(annotations(), object);
}

void SemanticAnalysisVisitor::visit(ast::MemberAccess& expression) {
    expression.getBase()->accept(*this);
    if (!expression.getBase()->hasResult(annotations()) || !expression.getBase()->hasExpressionType()) {
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
    // Hold that address in a pointer-sized result temp. Using the array type
    // (e.g. char[1]) made emitLoad sign-extend 1 byte of the pointer
    // (git archive: *header.typeflag = TYPEFLAG_REG SEGV).
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
