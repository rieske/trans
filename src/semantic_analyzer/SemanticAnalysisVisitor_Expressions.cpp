#include "SizeofOffsetof.h"
#include "SemanticAnalysisVisitorInternal.h"
#include "AggregateInitSink.h"
#include "InitializerLowering.h"

namespace semantic_analyzer {

namespace {

// Resolve AddressBaseMode + base symbol name together for Field/Index plans.
symbols::AddressBaseResolved resolveArrayBase(ast::Expression* base, bool baseIsArrayObject,
        symbols::AnnotationStore& store) {
    if (!baseIsArrayObject) {
        return { symbols::AddressBaseMode::PointerValue, base->result(store)->getName() };
    }
    // Nested a[i][j] / member-array row: address in Lvalue, not loaded Result.
    if (auto* lv = base->lvalueAnnotation(store)) {
        return { symbols::AddressBaseMode::PointerValue, lv->getName() };
    }
    return { symbols::AddressBaseMode::LeaObject, base->result(store)->getName() };
}

symbols::AddressBaseResolved resolveFieldBase(ast::Expression* base, bool isArrow,
        symbols::AnnotationStore& store) {
    if (isArrow) {
        // Arrow: base Result holds the pointer (not Lvalue of a prior member).
        return { symbols::AddressBaseMode::PointerValue, base->result(store)->getName() };
    }
    if (auto* lv = base->lvalueAnnotation(store)) {
        // Nested dot a[i].m / s.a.b: Lvalue is the base object address.
        return { symbols::AddressBaseMode::PointerValue, lv->getName() };
    }
    return { symbols::AddressBaseMode::LeaObject, base->result(store)->getName() };
}

} // namespace





void SemanticAnalysisVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    // Operand failed to resolve (e.g. undeclared identifier) — error already reported.
    if (!arrayAccess.getLeftOperand()->hasResult(store_) || !arrayAccess.getRightOperand()->hasResult(store_)) {
        return;
    }

    const type::ArraySubscriptInfo sub = type::arraySubscriptInfo(
            arrayAccess.leftOperandType(),
            arrayAccess.getLeftOperand()->valueType(store_));
    if (!sub.valid()) {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
        return;
    }
    // Index plan: SA owns AddressBaseResolved (mode + name).
    const auto base = resolveArrayBase(arrayAccess.getLeftOperand(), sub.baseIsArray, store_);
    symbols::IndexPlan indexPlan {
            arrayAccess.getLeftOperand(),
            arrayAccess.getRightOperand(),
            sub.elementStride,
            base };
    store_.setAddressPlan(&arrayAccess, symbols::AddressPlan { indexPlan });

    // Lvalue is the address of the element. When the element is itself an array
    // (multi-dim a[i] → T[N]), mirror MemberAccess: expressionType stays T[N]
    // for sizeof, value/lvalue is T* (&row[0]) — not T(*)[N], which would scale
    // a[i]+1 by sizeof(T[N]) and break git topath[i] / row decay.
    if (sub.elementType.isArray()) {
        auto addrSym = symbolTable.createTemporarySymbol(sub.elementType.decayArray());
        store_.setValue(&arrayAccess, symbols::ValueSlot::Lvalue, addrSym);
        arrayAccess.setAggregateAddressResult(store_, addrSym, sub.elementType);
    } else {
        store_.setValue(&arrayAccess, symbols::ValueSlot::Lvalue,
                symbolTable.createTemporarySymbol(type::pointer(sub.elementType)));
        arrayAccess.setTypeAndResult(store_, symbolTable.createTemporarySymbol(sub.elementType));
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    long enumValue = 0;
    if (symbolTable.hasEnumConstant(identifier.getIdentifier())) {
        enumValue = symbolTable.getEnumConstant(identifier.getIdentifier());
        identifier.setFoldedConstant(enumValue);
        identifier.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::signedInteger()));
    } else if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        auto entry = symbolTable.lookup(identifier.getIdentifier());
        // Only treat as designator if this lookup resolves to the function itself
        // (not a local that shadows a function of the same name).
        if (entry.getType().isFunction() && symbolTable.hasFunction(identifier.getIdentifier())) {
            auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
            type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
            type::Type ptrType = type::pointer(fnType);
            identifier.setFunctionDesignatorResult(store_, symbolTable.createTemporarySymbol(ptrType),
                    functionEntry.getName());
        } else {
            identifier.setTypeAndResult(store_, entry);
        }
    } else if (symbolTable.hasFunction(identifier.getIdentifier())) {
        // Function designator decays to pointer-to-function when used as a value.
        auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
        type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
        type::Type ptrType = type::pointer(fnType);
        identifier.setFunctionDesignatorResult(store_, symbolTable.createTemporarySymbol(ptrType),
                functionEntry.getName());
    } else {
        semanticError("symbol `" + identifier.getIdentifier() + "` is not defined", identifier.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setTypeAndResult(store_, symbolTable.createTemporarySymbol(constant.expressionType()));
}

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    stringLiteral.setTypeAndResult(store_, symbolTable.createTemporarySymbol(stringLiteral.expressionType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(store_)) {
        return;
    }

    auto operandSymbol = *expression.getOperandExpression()->result(store_);
    expression.setTypeAndResult(store_, operandSymbol);

    auto preOperationSymbolName = operandSymbol.getName() + "_pre";
    symbolTable.insertSymbol(preOperationSymbolName, operandSymbol.getType(), operandSymbol.getContext());
    store_.setValue(&expression, symbols::ValueSlot::PreOperation, symbolTable.lookup(preOperationSymbolName));

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(store_)) {
        return;
    }

    expression.setTypeAndResult(store_, *expression.getOperandExpression()->result(store_));

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (op == OperatorKind::Sizeof) {
        // Resolve operand type; do not rely on runtime evaluation of the operand.
        expression.visitOperand(*this);
        applySizeofResult(expression, store_, symbolTable);
        return;
    }

    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(store_)) {
        return;
    }

    switch (op) {
    case OperatorKind::AddressOf: {
        auto* operand = expression.getOperandExpression();
        // Offsetof fold extracted to SizeofOffsetof (FieldPlan + null arrow base).
        if (auto* member = dynamic_cast<ast::MemberAccess*>(operand)) {
            if (!store_.addressPlan(member)) {
                break; // member visit failed; error already reported
            }
            if (tryApplyOffsetofFold(expression, operand, store_, symbolTable)) {
                break;
            }
        }
        // ArrayAccess / MemberAccess already store Index/Field plans in their visits.
        // Only fill a plan when the operand has none yet.
        if (!store_.addressPlan(operand)) {
            if (operand->holdsFunctionDesignator()) {
                // FunctionDesignatorPlan already set by setFunctionDesignatorResult.
            } else if (operand->lvalueAnnotation(store_)) {
                store_.setAddressPlan(operand, symbols::AddressPlan { symbols::LvaluePlan {} });
            } else {
                store_.setAddressPlan(operand, symbols::AddressPlan { symbols::ResultAddressOfPlan {} });
            }
        }
        expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
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
                expression.setTypeAndResult(store_, *expression.getOperandExpression()->result(store_));
            } else {
                expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(pointee));
                store_.setValue(&expression, symbols::ValueSlot::Lvalue, symbolTable.createTemporarySymbol(operandType));
            }
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().to_string(), expression.getContext());
        }
        break;
    }
    case OperatorKind::Add:
        expression.setTypeAndResult(store_, *expression.getOperandExpression()->result(store_));
        break;
    case OperatorKind::Sub:
        expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case OperatorKind::LogicalNot:
        expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::signedInteger()));
        store_.setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
        store_.setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
        break;
    case OperatorKind::BitNot:
        // Integer promotions apply to ~ (C 6.5.3.3).
        expression.setTypeAndResult(store_, 
                symbolTable.createTemporarySymbol(type::integerPromote(expression.operandType())));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);

    if (!expression.getOperandExpression()->hasResult(store_)) {
        return;
    }

    const translation_unit::Context castCtx = expression.getContext();
    type::Type target = resolveTypeSpecifier(expression.getTypeSpecifier(), *this, store_,
            [this](std::string msg, const translation_unit::Context& ctx) {
                semanticError(std::move(msg), ctx);
            },
            &castCtx);
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
    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(target));
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

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    // Pointer +/- integer: scale the integer by pointee size (C 6.5.6).
    // Match pointer-index stride so p+1 agrees with p[1] (System V natural).
    // Use value types after decay (member arrays: expressionType T[N], valueType pointer).
    type::Type resultType = expression.getLeftOperand()->valueType(store_);
    bool pointerArithmetic = false;
    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        auto pointerScale = [](const type::Type& ptrType) {
            type::Type elem = ptrType.dereference();
            int scale = elem.getSize();
            if (scale < 1) {
                scale = 1;
            }
            return scale;
        };
        const type::Type lt = expression.getLeftOperand()->valueType(store_);
        const type::Type rt = expression.getRightOperand()->valueType(store_);
        if (lt.isPointer() && !rt.isPointer()) {
            resultType = lt;
            pointerArithmetic = true;
            int scale = pointerScale(lt);
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                store_.setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerScalePlan { scale, scaleTemp.getName(), true } });
            }
        } else if (op == OperatorKind::Add && rt.isPointer() && !lt.isPointer()) {
            resultType = rt;
            pointerArithmetic = true;
            int scale = pointerScale(rt);
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                store_.setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerScalePlan { scale, scaleTemp.getName(), false } });
            }
        } else if (op == OperatorKind::Sub && lt.isPointer() && rt.isPointer()) {
            // ptrdiff_t in elements, not bytes (git: dir->nr = dst - dir->entries).
            resultType = type::signedLong();
            pointerArithmetic = true;
            int scale = pointerScale(lt);
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                store_.setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerDifferencePlan { scale, scaleTemp.getName() } });
            }
        }
    }

    if (!pointerArithmetic) {
        requireArithmeticCompatible(
                expression.getLeftOperand()->valueType(store_),
                expression.getRightOperand()->valueType(store_),
                expression.getContext());
        // Usual arithmetic conversions (C 6.3.1.8). Use value types after decay
        // (member arrays keep expressionType T[N] but valueType is pointer).
        resultType = type::usualArithmeticResult(
                expression.getLeftOperand()->valueType(store_), expression.getRightOperand()->valueType(store_));
    }

    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    if (type::isIntegral(expression.getRightOperand()->valueType(store_))) {
        // Result type is the promoted left operand (C 6.5.7).
        expression.setTypeAndResult(store_, 
                symbolTable.createTemporarySymbol(type::integerPromote(expression.getLeftOperand()->valueType(store_))));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    // Value operands: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getLeftOperand());
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(store_),
            expression.getRightOperand()->valueType(store_),
            expression.getContext());

    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::signedInteger()));
    store_.setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
    store_.setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(store_),
            expression.getRightOperand()->valueType(store_),
            expression.getContext());
    if (!type::isIntegral(expression.getLeftOperand()->valueType(store_))
            || !type::isIntegral(expression.getRightOperand()->valueType(store_))) {
        semanticError("invalid operands to binary bitwise operator", expression.getContext());
        return;
    }

    // Usual arithmetic conversions include integer promotions (C 6.5.10-12).
    type::Type resultType = type::usualArithmeticResult(
            expression.getLeftOperand()->valueType(store_), expression.getRightOperand()->valueType(store_));
    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(store_),
            expression.getRightOperand()->valueType(store_),
            expression.getContext());

    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::signedInteger()));
    store_.setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    requireValueCompatible(
            expression.getLeftOperand()->valueType(store_),
            expression.getRightOperand()->valueType(store_),
            expression.getContext());

    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(type::signedInteger()));
    store_.setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    // Value arms: visit + array-to-pointer decay (C 6.3.2.1 / 6.5.15).
    analyzeAsRvalue(expression.getTrueExpression());
    analyzeAsRvalue(expression.getFalseExpression());

    if (!expression.getCondition()->hasResult(store_)
            || !expression.getTrueExpression()->hasResult(store_)
            || !expression.getFalseExpression()->hasResult(store_)) {
        return;
    }

    // Use value types after decay, not expressionType(). Member arrays keep
    // expressionType as T[N] for sizeof, but their result symbol is already
    // &member[0] (pointer). Using expressionType makes the ternary result an
    // array temporary; assignment/call-arg decay then takes &stack_temp
    // (git xdiff: buf = func_line ? func_line->buf : dummy - hunk funcname NUL).
    const type::Type trueTy = expression.getTrueExpression()->result(store_)->getType();
    const type::Type falseTy = expression.getFalseExpression()->result(store_)->getType();
    type::Type resultType = trueTy;
    if (!trueTy.isVoid() && !falseTy.isVoid()) {
        requireValueCompatible(trueTy, falseTy, expression.getContext());
    } else {
        // Void ternary used as a statement (assert macros: cond ? (void)0 : die()).
        resultType = type::voidType();
    }
    expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(resultType));
    store_.setLabel(&expression, symbols::LabelSlot::Truthy, symbolTable.newLabel());
    store_.setLabel(&expression, symbols::LabelSlot::Falsy, symbolTable.newLabel());
    store_.setLabel(&expression, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this); // lvalue: no decay
    // RHS value context: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }

    if (expression.isLval()) {

        // requireProductAssignable(dest, source): same order as productAssignFrom.
        // Value types after RHS decay.
        requireProductAssignable(
                expression.getLeftOperand()->valueType(store_),
                expression.getRightOperand()->valueType(store_),
                expression.getContext());

        // Pointer += / -= integer: scale like p = p + n (C 6.5.16.2 / 6.5.6).
        // git xdiff: changed += 1 then free(changed - 1) requires matching strides.
        using ast::OperatorKind;
        const OperatorKind op = expression.getOperator()->getKind();
        if ((op == OperatorKind::AddAssign || op == OperatorKind::SubAssign)
                && expression.getLeftOperand()->valueType(store_).isPointer()
                && !expression.getRightOperand()->valueType(store_).isPointer()) {
            type::Type elem = expression.getLeftOperand()->valueType(store_).dereference();
            int scale = elem.getSize();
            if (scale < 1) {
                scale = 1;
            }
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                // Compound assign always scales RHS (pointer on left).
                store_.setPointerArithPlan(&expression, symbols::PointerArithPlan {
                        symbols::PointerScalePlan { scale, scaleTemp.getName(), true } });
            }
        }

        expression.setTypeAndResult(store_, *expression.getLeftOperand()->result(store_));
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.getLeftOperand()->hasResult(store_) || !expression.getRightOperand()->hasResult(store_)) {
        return;
    }
    // Comma operator: value and type of the right operand
    expression.setTypeAndResult(store_, *expression.getRightOperand()->result(store_));
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
    type::Type objectType = resolveTypeSpecifier(expression.getTypeSpecifier(), *this, store_,
            [this](std::string msg, const translation_unit::Context& ctx) {
                semanticError(std::move(msg), ctx);
            },
            &litCtx);
    auto* initExpr = expression.getInitializer();
    AggregateInitHost host {
        store_,
        [this](std::string msg, const translation_unit::Context& ctx) {
            semanticError(std::move(msg), ctx);
        }
    };
    // Incomplete arrays are completed only inside lowerToFieldInits; size the temp from that.
    type::Type completed = objectType;
    if (initExpr) {
        completed = lowerToFieldInits(objectType, initExpr, symbolTable, host,
                [&](symbols::StructFieldInit init) {
                    store_.addStructFieldInit(&expression, std::move(init));
                });
    }
    auto object = symbolTable.createTemporarySymbol(completed);
    store_.setValue(&expression, symbols::ValueSlot::Object, object);
    expression.setTypeAndResult(store_, object);
}

void SemanticAnalysisVisitor::visit(ast::MemberAccess& expression) {
    expression.getBase()->accept(*this);
    type::Type baseType = expression.getBase()->expressionType();
    type::Type structType = baseType;
    if (expression.isArrow()) {
        if (!baseType.isPointer()) {
            semanticError("base of '->' is not a pointer", expression.getContext());
            return;
        }
        structType = baseType.dereference();
    }
    if (!structType.isRecord()) {
        semanticError("request for member in non-struct", expression.getContext());
        return;
    }
    type::Type memberTy = type::signedInteger();
    int offset = 0;
    if (!structType.memberType(expression.getMemberName(), memberTy) ||
            !structType.memberOffset(expression.getMemberName(), offset)) {
        semanticError("struct has no member named `" + expression.getMemberName() + "`", expression.getContext());
        return;
    }
    // Field plan: SA owns AddressBaseResolved (mode + name).
    const auto base = resolveFieldBase(expression.getBase(), expression.isArrow(), store_);
    symbols::FieldPlan fieldPlan { expression.getBase(), offset, base };
    store_.setAddressPlan(&expression, symbols::AddressPlan { fieldPlan });
    // Array members: field address is pointer-to-element (&arr[0]), not
    // pointer-to-array. Decay sites reuse this lvalue; pointer-to-array would
    // make s.buf + i scale by sizeof(buf) (git sha1dc: ctx->buffer + left).
    // Hold that address in a pointer-sized result temp. Using the array type
    // (e.g. char[1]) made emitLoad sign-extend 1 byte of the pointer
    // (git archive: *header.typeflag = TYPEFLAG_REG SEGV).
    if (memberTy.isArray()) {
        auto addrSym = symbolTable.createTemporarySymbol(memberTy.decayArray());
        store_.setValue(&expression, symbols::ValueSlot::Lvalue, addrSym);
        expression.setAggregateAddressResult(store_, addrSym, memberTy);
    } else {
        expression.setTypeAndResult(store_, symbolTable.createTemporarySymbol(memberTy));
        store_.setValue(&expression, symbols::ValueSlot::Lvalue,
                symbolTable.createTemporarySymbol(type::pointer(memberTy)));
    }
}

} // namespace semantic_analyzer
