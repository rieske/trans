#include "SemanticAnalysisVisitorInternal.h"

namespace semantic_analyzer {




void SemanticAnalysisVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    // Operand failed to resolve (e.g. undeclared identifier) — error already reported.
    if (!arrayAccess.hasLeftOperandSymbol() || !arrayAccess.hasRightOperandSymbol()) {
        return;
    }

    auto type = arrayAccess.leftOperandType();
    const type::ArraySubscriptInfo sub = type::arraySubscriptInfo(type);
    if (sub.elementStride < 1) {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
        return;
    }
    type::Type elementType = sub.elementType;
    arrayAccess.setBaseIsArray(sub.baseIsArray);
    arrayAccess.setElementSize(sub.elementStride);
    // Lvalue is the address of the element. When the element is itself an array
    // (multi-dim a[i] → T[N]), mirror MemberAccess: expressionType stays T[N]
    // for sizeof, value/lvalue is T* (&row[0]) — not T(*)[N], which would scale
    // a[i]+1 by sizeof(T[N]) and break git topath[i] / row decay.
    if (elementType.isArray()) {
        auto addrSym = symbolTable.createTemporarySymbol(elementType.decayArray());
        arrayAccess.setLvalue(addrSym);
        arrayAccess.setType(elementType);
        arrayAccess.setResultSymbol(addrSym);
    } else {
        arrayAccess.setLvalue(symbolTable.createTemporarySymbol(type::pointer(elementType)));
        arrayAccess.setTypedResult(symbolTable.createTemporarySymbol(elementType));
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    long enumValue = 0;
    if (symbolTable.hasEnumConstant(identifier.getIdentifier())) {
        enumValue = symbolTable.getEnumConstant(identifier.getIdentifier());
        identifier.setFoldedConstant(enumValue);
        identifier.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
    } else if (scanner::EnumConstantRegistry::lookup(identifier.getIdentifier(), enumValue)) {
        // Nested enums (e.g. as struct member types) never go through
        // DeclarationSpecifiers, so they are not in the symbol table - only in the
        // parse-time EnumConstantRegistry. Import and fold them here.
        symbolTable.defineEnumConstant(identifier.getIdentifier(), enumValue);
        identifier.setFoldedConstant(enumValue);
        identifier.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
    } else if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        auto entry = symbolTable.lookup(identifier.getIdentifier());
        // Only treat as designator if this lookup resolves to the function itself
        // (not a local that shadows a function of the same name).
        if (entry.getType().isFunction() && symbolTable.hasFunction(identifier.getIdentifier())) {
            auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
            type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
            type::Type ptrType = type::pointer(fnType);
            identifier.setTypedResult(symbolTable.createTemporarySymbol(ptrType));
            identifier.setFunctionDesignator(functionEntry.getName());
        } else {
            identifier.setTypedResult(entry);
        }
    } else if (symbolTable.hasFunction(identifier.getIdentifier())) {
        // Function designator decays to pointer-to-function when used as a value.
        auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
        type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
        type::Type ptrType = type::pointer(fnType);
        identifier.setTypedResult(symbolTable.createTemporarySymbol(ptrType));
        identifier.setFunctionDesignator(functionEntry.getName());
    } else {
        semanticError("symbol `" + identifier.getIdentifier() + "` is not defined", identifier.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setTypedResult(symbolTable.createTemporarySymbol(constant.expressionType()));
}

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    stringLiteral.setTypedResult(symbolTable.createTemporarySymbol(stringLiteral.expressionType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setType(expression.operandType());
    auto operandSymbol = *expression.operandSymbol();
    expression.setTypedResult(operandSymbol);

    auto preOperationSymbolName = operandSymbol.getName() + "_pre";
    symbolTable.insertSymbol(preOperationSymbolName, operandSymbol.getType(), operandSymbol.getContext());
    expression.setPreOperationSymbol(symbolTable.lookup(preOperationSymbolName));

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setType(expression.operandType());
    expression.setResultSymbol(*expression.operandSymbol());

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (op == OperatorKind::Sizeof) {
        // Resolve operand type; do not rely on runtime evaluation of the operand.
        // C: sizeof yields size_t (unsigned); signed breaks unsigned_add_overflows.
        expression.visitOperand(*this);
        if (!expression.hasOperandSymbol()) {
            expression.setTypedResult(symbolTable.createTemporarySymbol(type::unsignedLong()));
            return;
        }
        // sizeof policy: string literals use lexical array length (TypeQuery).
        if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(
                    expression.getOperandExpression())) {
            expression.setSizeofValue(type::sizeofStringLiteralTokenBytes(strLit->getValue()));
        } else {
            expression.setSizeofValue(expression.operandType().getSize());
        }
        expression.setTypedResult(symbolTable.createTemporarySymbol(type::unsignedLong()));
        return;
    }

    expression.visitOperand(*this);

    if (!expression.hasOperandSymbol()) {
        return;
    }

    switch (op) {
    case OperatorKind::AddressOf: {
        // Offsetof fold: product_approx policy for &((T*)0)->member.
        if (auto* member = dynamic_cast<ast::MemberAccess*>(expression.getOperandExpression())) {
            if (auto off = product_approx::foldOffsetofArrowFromNull(member)) {
                expression.setSizeofValue(*off);
                expression.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
                break;
            }
        }
        expression.setTypedResult(symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
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
                expression.setTypedResult(*expression.operandSymbol());
            } else {
                expression.setTypedResult(symbolTable.createTemporarySymbol(pointee));
                expression.setLvalueSymbol(symbolTable.createTemporarySymbol(operandType));
            }
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().to_string(), expression.getContext());
        }
        break;
    }
    case OperatorKind::Add:
        expression.setTypedResult(*expression.operandSymbol());
        break;
    case OperatorKind::Sub:
        expression.setTypedResult(symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case OperatorKind::LogicalNot:
        expression.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
        expression.setTruthyLabel(symbolTable.newLabel());
        expression.setFalsyLabel(symbolTable.newLabel());
        break;
    case OperatorKind::BitNot:
        // Integer promotions apply to ~ (C 6.5.3.3).
        expression.setTypedResult(
                symbolTable.createTemporarySymbol(type::integerPromote(expression.operandType())));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);

    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setTypedResult(symbolTable.createTemporarySymbol(expression.getType().getType()));
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

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    requireProductAssignable(
            expression.leftValueType(),
            expression.rightValueType(),
            expression.getContext());

    // Pointer +/- integer: scale the integer by pointee size (C 6.5.6).
    // Match pointer-index stride so p+1 agrees with p[1] (System V natural).
    // Use value types after decay (member arrays: expressionType T[N], valueType pointer).
    type::Type resultType = expression.leftValueType();
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
        const type::Type lt = expression.leftValueType();
        const type::Type rt = expression.rightValueType();
        if (lt.isPointer() && !rt.isPointer()) {
            resultType = lt;
            pointerArithmetic = true;
            int scale = pointerScale(lt);
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                expression.setPointerArithmetic(scale, scaleTemp.getName(), true);
            }
        } else if (op == OperatorKind::Add && rt.isPointer() && !lt.isPointer()) {
            resultType = rt;
            pointerArithmetic = true;
            int scale = pointerScale(rt);
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                expression.setPointerArithmetic(scale, scaleTemp.getName(), false);
            }
        } else if (op == OperatorKind::Sub && lt.isPointer() && rt.isPointer()) {
            // ptrdiff_t in elements, not bytes (git: dir->nr = dst - dir->entries).
            resultType = type::signedLong();
            pointerArithmetic = true;
            int scale = pointerScale(lt);
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                expression.setPointerDifference(scale, scaleTemp.getName());
            }
        }
    }

    if (!pointerArithmetic) {
        // Usual arithmetic conversions (C 6.3.1.8). Use value types after decay
        // (member arrays keep expressionType T[N] but valueType is pointer).
        resultType = type::usualArithmeticResult(
                expression.leftValueType(), expression.rightValueType());
    }

    expression.setTypedResult(symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    if (type::isIntegral(expression.rightValueType())) {
        // Result type is the promoted left operand (C 6.5.7).
        expression.setTypedResult(
                symbolTable.createTemporarySymbol(type::integerPromote(expression.leftValueType())));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    // Value operands: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getLeftOperand());
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    requireProductAssignable(
            expression.leftValueType(),
            expression.rightValueType(),
            expression.getContext());

    expression.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setTruthyLabel(symbolTable.newLabel());
    expression.setFalsyLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    requireProductAssignable(
            expression.leftValueType(),
            expression.rightValueType(),
            expression.getContext());

    // Usual arithmetic conversions include integer promotions (C 6.5.10-12).
    type::Type resultType = type::usualArithmeticResult(
            expression.leftValueType(), expression.rightValueType());
    expression.setType(resultType);
    expression.setResultSymbol(symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    requireProductAssignable(
            expression.leftValueType(),
            expression.rightValueType(),
            expression.getContext());

    expression.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    requireProductAssignable(
            expression.leftValueType(),
            expression.rightValueType(),
            expression.getContext());

    expression.setTypedResult(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    // Value arms: visit + array-to-pointer decay (C 6.3.2.1 / 6.5.15).
    analyzeAsRvalue(expression.getTrueExpression());
    analyzeAsRvalue(expression.getFalseExpression());

    if (!expression.getCondition()->hasResultSymbol()
            || !expression.getTrueExpression()->hasResultSymbol()
            || !expression.getFalseExpression()->hasResultSymbol()) {
        return;
    }

    // Use value types after decay, not expressionType(). Member arrays keep
    // expressionType as T[N] for sizeof, but their result symbol is already
    // &member[0] (pointer). Using expressionType makes the ternary result an
    // array temporary; assignment/call-arg decay then takes &stack_temp
    // (git xdiff: buf = func_line ? func_line->buf : dummy - hunk funcname NUL).
    const type::Type trueTy = expression.trueSymbol()->getType();
    const type::Type falseTy = expression.falseSymbol()->getType();
    type::Type resultType = trueTy;
    if (!trueTy.isVoid() && !falseTy.isVoid()) {
        requireProductAssignable(trueTy, falseTy, expression.getContext());
    } else {
        // Void ternary used as a statement (assert macros: cond ? (void)0 : die()).
        resultType = type::voidType();
    }
    expression.setTypedResult(symbolTable.createTemporarySymbol(resultType));
    expression.setTruthyLabel(symbolTable.newLabel());
    expression.setFalsyLabel(symbolTable.newLabel());
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this); // lvalue: no decay
    // RHS value context: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(expression.getRightOperand());

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    if (expression.isLval()) {

        // requireProductAssignable(source, dest): dest product-accepts source (LHS ← RHS).
        // Value types after RHS decay.
        requireProductAssignable(
                expression.rightValueType(),
                expression.leftValueType(),
                expression.getContext());

        // Pointer += / -= integer: scale like p = p + n (C 6.5.16.2 / 6.5.6).
        // git xdiff: changed += 1 then free(changed - 1) requires matching strides.
        using ast::OperatorKind;
        const OperatorKind op = expression.getOperator()->getKind();
        if ((op == OperatorKind::AddAssign || op == OperatorKind::SubAssign)
                && expression.leftValueType().isPointer()
                && !expression.rightValueType().isPointer()) {
            type::Type elem = expression.leftValueType().dereference();
            int scale = elem.getSize();
            if (scale < 1) {
                scale = 1;
            }
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                expression.setPointerArithmetic(scale, scaleTemp.getName());
            }
        }

        expression.setTypedResult(*expression.leftOperandSymbol());
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }
    // Comma operator: value and type of the right operand
    expression.setType(expression.rightOperandType());
    expression.setResultSymbol(*expression.rightOperandSymbol());
}

void SemanticAnalysisVisitor::visit(ast::Operator&) {
}

void SemanticAnalysisVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
}

void SemanticAnalysisVisitor::visit(ast::CompoundLiteralExpression& expression) {
    // Materialize (type){ init } as a stack temporary; expand like local brace init.
    if (expression.getInitializer()) {
        expression.getInitializer()->accept(*this);
    }

    type::Type objectType = expression.getTypeSpecifier().getType();
    auto* initExpr = expression.getInitializer();
    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(initExpr)) {
        if (objectType.isArray() && objectType.getArraySize() == 0) {
            objectType = completeArrayTypeFromList(objectType, list);
        }
    } else if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(initExpr)) {
        if (objectType.isArray() && objectType.getArraySize() == 0) {
            objectType = completeArrayTypeFromString(objectType, strLit);
        }
    }

    auto object = symbolTable.createTemporarySymbol(objectType);
    expression.setObjectSymbol(object);
    expression.setType(objectType);
    expression.setResultSymbol(object);

    if (!initExpr) {
        return;
    }
    lowerToFieldInits(objectType, initExpr, symbolTable,
            [&](ast::StructFieldInit init) {
                expression.addStructFieldInit(std::move(init));
            });
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
    expression.setMemberOffset(offset);
    expression.setBaseResultSymbol(*expression.getBase()->getResultSymbol());
    // Array members: field address is pointer-to-element (&arr[0]), not
    // pointer-to-array. Decay sites reuse this lvalue; pointer-to-array would
    // make s.buf + i scale by sizeof(buf) (git sha1dc: ctx->buffer + left).
    // Hold that address in a pointer-sized result temp. Using the array type
    // (e.g. char[1]) made emitLoad sign-extend 1 byte of the pointer
    // (git archive: *header.typeflag = TYPEFLAG_REG SEGV).
    if (memberTy.isArray()) {
        auto addrSym = symbolTable.createTemporarySymbol(memberTy.decayArray());
        expression.setFieldAddressSymbol(addrSym);
        // expressionType = array first; setResultSymbol preserves it (value = pointer).
        expression.setType(memberTy);
        expression.setResultSymbol(addrSym);
    } else {
        expression.setTypedResult(symbolTable.createTemporarySymbol(memberTy));
        expression.setFieldAddressSymbol(
                symbolTable.createTemporarySymbol(type::pointer(memberTy)));
    }
}

} // namespace semantic_analyzer
