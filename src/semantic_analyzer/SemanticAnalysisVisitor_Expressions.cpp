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
    type::Type elementType = type::voidType();
    int elementStride = 8;
    if (type.isArray()) {
        elementType = type.getElementType();
        elementStride = type.getElementStride();
        arrayAccess.setBaseIsArray(true);
    } else if (type.isPointer()) {
        elementType = type.dereference();
        // System V natural element size for p[i] (int* strides by 4).
        elementStride = elementType.getSize();
        if (elementStride < 1) {
            elementStride = 1;
        }
        if (elementType.isArray()) {
            elementStride = elementType.getSize();
        }
        arrayAccess.setBaseIsArray(false);
    } else {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
        return;
    }
    arrayAccess.setElementSize(elementStride);
    // Lvalue is the address of the element (pointer), result is the loaded element.
    arrayAccess.setLvalue(symbolTable.createTemporarySymbol(type::pointer(elementType)));
    arrayAccess.setResultSymbol(symbolTable.createTemporarySymbol(elementType));
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    long enumValue = 0;
    if (symbolTable.hasEnumConstant(identifier.getIdentifier())) {
        enumValue = symbolTable.getEnumConstant(identifier.getIdentifier());
        identifier.setFoldedConstant(enumValue);
        identifier.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    } else if (scanner::EnumConstantRegistry::lookup(identifier.getIdentifier(), enumValue)) {
        // Nested enums (e.g. as struct member types) never go through
        // DeclarationSpecifiers, so they are not in the symbol table - only in the
        // parse-time EnumConstantRegistry. Import and fold them here.
        symbolTable.defineEnumConstant(identifier.getIdentifier(), enumValue);
        identifier.setFoldedConstant(enumValue);
        identifier.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    } else if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        auto entry = symbolTable.lookup(identifier.getIdentifier());
        // Only treat as designator if this lookup resolves to the function itself
        // (not a local that shadows a function of the same name).
        if (entry.getType().isFunction() && symbolTable.hasFunction(identifier.getIdentifier())) {
            auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
            type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
            type::Type ptrType = type::pointer(fnType);
            identifier.setResultSymbol(symbolTable.createTemporarySymbol(ptrType));
            identifier.setFunctionDesignator(functionEntry.getName());
        } else {
            identifier.setResultSymbol(entry);
        }
    } else if (symbolTable.hasFunction(identifier.getIdentifier())) {
        // Function designator decays to pointer-to-function when used as a value.
        auto functionEntry = symbolTable.findFunction(identifier.getIdentifier());
        type::Type fnType = type::function(functionEntry.returnType(), functionEntry.arguments());
        type::Type ptrType = type::pointer(fnType);
        identifier.setResultSymbol(symbolTable.createTemporarySymbol(ptrType));
        identifier.setFunctionDesignator(functionEntry.getName());
    } else {
        semanticError("symbol `" + identifier.getIdentifier() + "` is not defined", identifier.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setResultSymbol(symbolTable.createTemporarySymbol(constant.getType()));
}

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    stringLiteral.setResultSymbol(symbolTable.createTemporarySymbol(stringLiteral.getType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setType(expression.operandType());
    auto operandSymbol = *expression.operandSymbol();
    expression.setResultSymbol(operandSymbol);

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
            expression.setResultSymbol(symbolTable.createTemporarySymbol(type::unsignedLong()));
            return;
        }
        // String literals have type char[N] for sizeof (C 6.4.5 / 6.5.3.4); the
        // AST types them as const char* for decay in other contexts. git ctype
        // tests use ARRAY_SIZE("...") which is sizeof(literal)/sizeof(char).
        if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(
                    expression.getOperandExpression())) {
            expression.setSizeofValue(util::stringLiteralArrayLength(strLit->getValue()));
        } else {
            expression.setSizeofValue(expression.operandType().getSize());
        }
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::unsignedLong()));
        return;
    }

    expression.visitOperand(*this);

    if (!expression.hasOperandSymbol()) {
        return;
    }

    switch (op) {
    case OperatorKind::AddressOf: {
        // Fold &((T *)0)->member (from __builtin_offsetof rewrite) to a field offset.
        if (auto* member = dynamic_cast<ast::MemberAccess*>(expression.getOperandExpression())) {
            long baseVal = 1;
            if (member->isArrow() && member->getBase()->evaluateConstant(baseVal) && baseVal == 0) {
                expression.setSizeofValue(member->getMemberOffset());
                expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
                break;
            }
        }
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
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
                expression.setResultSymbol(*expression.operandSymbol());
            } else {
                expression.setResultSymbol(symbolTable.createTemporarySymbol(pointee));
                expression.setLvalueSymbol(symbolTable.createTemporarySymbol(operandType));
            }
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().to_string(), expression.getContext());
        }
        break;
    }
    case OperatorKind::Add:
        expression.setResultSymbol(*expression.operandSymbol());
        break;
    case OperatorKind::Sub:
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case OperatorKind::LogicalNot:
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
        expression.setTruthyLabel(symbolTable.newLabel());
        expression.setFalsyLabel(symbolTable.newLabel());
        break;
    case OperatorKind::BitNot:
        // Integer promotions apply to ~ (C 6.5.3.3).
        expression.setResultSymbol(
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

    expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.getType().getType()));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    // Array-to-pointer decay for +/- (C 6.3.2.1 / 6.5.6): commands + i, i + commands.
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        decayArrayInPlace(expression.getLeftOperand(), symbolTable);
        decayArrayInPlace(expression.getRightOperand(), symbolTable);
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    // Pointer +/- integer: scale the integer by pointee size (C 6.5.6).
    // Match pointer-index stride so p+1 agrees with p[1] (System V natural).
    //
    // Use result-symbol types after decay, not expression getType(). Member
    // arrays keep expression type as T[N] for sizeof, but their result symbol
    // is already &member[0] (pointer). Using getType() misses that and makes
    // `ctx->buffer + left` an array-typed temporary; call-arg decay then takes
    // &stack_temp instead of the pointer (git sha1dc: memcpy(ctx->buffer+left)).
    type::Type resultType = expression.leftOperandSymbol()->getType();
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
        const type::Type lt = expression.leftOperandSymbol()->getType();
        const type::Type rt = expression.rightOperandSymbol()->getType();
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
        // Usual arithmetic conversions (C 6.3.1.8): floating -> double; otherwise
        // integer promotions then the wider (and unsigned-over-signed) type wins.
        // Critical for 1 + (uintptr_t)p (git cbtree tagged pointers).
        const bool leftFloat = expression.leftOperandType().isPrimitive()
                && expression.leftOperandType().getPrimitive().isFloating();
        const bool rightFloat = expression.rightOperandType().isPrimitive()
                && expression.rightOperandType().getPrimitive().isFloating();
        if (leftFloat || rightFloat) {
            resultType = type::doubleFloating();
        } else {
            type::Type leftP = type::integerPromote(expression.leftOperandType());
            type::Type rightP = type::integerPromote(expression.rightOperandType());
            resultType = leftP;
            if (rightP.getSize() > leftP.getSize()) {
                resultType = rightP;
            } else if (rightP.getSize() == leftP.getSize() && rightP.isPrimitive() && leftP.isPrimitive()
                    && !rightP.getPrimitive().isSigned() && leftP.getPrimitive().isSigned()) {
                resultType = rightP;
            }
        }
    }

    expression.setResultSymbol(symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    if (expression.rightOperandType().isPrimitive() && !expression.rightOperandType().getPrimitive().isFloating()) {
        // Result type is the promoted left operand (C 6.5.7).
        expression.setResultSymbol(
                symbolTable.createTemporarySymbol(type::integerPromote(expression.leftOperandType())));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    // Array-to-pointer decay for relational operands (C 6.3.2.1): p == arr, arr != p.
    decayArrayInPlace(expression.getLeftOperand(), symbolTable);
    decayArrayInPlace(expression.getRightOperand(), symbolTable);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setTruthyLabel(symbolTable.newLabel());
    expression.setFalsyLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    // Usual arithmetic conversions include integer promotions (C 6.5.10-12).
    // Prefer the wider promoted side so char|char and (char<<n)|char stay int.
    type::Type leftP = type::integerPromote(expression.leftOperandType());
    type::Type rightP = type::integerPromote(expression.rightOperandType());
    type::Type resultType = leftP;
    if (rightP.getSize() > leftP.getSize()) {
        resultType = rightP;
    } else if (rightP.getSize() == leftP.getSize() && rightP.isPrimitive() && leftP.isPrimitive()
            && !rightP.getPrimitive().isSigned() && leftP.getPrimitive().isSigned()) {
        resultType = rightP;
    }
    expression.setType(resultType);
    expression.setResultSymbol(symbolTable.createTemporarySymbol(resultType));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    expression.visitTrueExpression(*this);
    expression.visitFalseExpression(*this);

    if (!expression.getCondition()->hasResultSymbol()
            || !expression.getTrueExpression()->hasResultSymbol()
            || !expression.getFalseExpression()->hasResultSymbol()) {
        return;
    }

    // Array-to-pointer decay for ?: arms (C 6.3.2.1 / 6.5.15).
    decayArrayInPlace(expression.getTrueExpression(), symbolTable);
    decayArrayInPlace(expression.getFalseExpression(), symbolTable);

    // Use result-symbol types after decay, not expression getType(). Member
    // arrays keep getType() as T[N] for sizeof, but their result symbol is
    // already &member[0] (pointer). Using getType() makes the ternary result an
    // array temporary; assignment/call-arg decay then takes &stack_temp
    // (git xdiff: buf = func_line ? func_line->buf : dummy - hunk funcname NUL).
    const type::Type trueTy = expression.trueSymbol()->getType();
    const type::Type falseTy = expression.falseSymbol()->getType();
    type::Type resultType = trueTy;
    if (!trueTy.isVoid() && !falseTy.isVoid()) {
        typeCheck(trueTy, falseTy, expression.getContext());
    } else {
        // Void ternary used as a statement (assert macros: cond ? (void)0 : die()).
        resultType = type::voidType();
    }
    expression.setResultSymbol(symbolTable.createTemporarySymbol(resultType));
    expression.setTruthyLabel(symbolTable.newLabel());
    expression.setFalsyLabel(symbolTable.newLabel());
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    if (expression.isLval()) {
        // Array-to-pointer decay on RHS (C 6.3.2.1): p = m where m is T[N] assigns &m[0].
        decayArrayInPlace(expression.getRightOperand(), symbolTable);

        // typeCheck(from, to) requires to.canAssignFrom(from): destination (LHS)
        // must accept the source (RHS).
        typeCheck(
                expression.rightOperandType(),
                expression.leftOperandType(),
                expression.getContext());

        // Pointer += / -= integer: scale like p = p + n (C 6.5.16.2 / 6.5.6).
        // git xdiff: changed += 1 then free(changed - 1) requires matching strides.
        using ast::OperatorKind;
        const OperatorKind op = expression.getOperator()->getKind();
        if ((op == OperatorKind::AddAssign || op == OperatorKind::SubAssign)
                && expression.leftOperandType().isPointer()
                && !expression.rightOperandType().isPointer()) {
            type::Type elem = expression.leftOperandType().dereference();
            int scale = elem.getSize();
            if (scale < 1) {
                scale = 1;
            }
            if (scale > 1) {
                auto scaleTemp = symbolTable.createTemporarySymbol(type::signedLong());
                expression.setPointerArithmetic(scale, scaleTemp.getName());
            }
        }

        expression.setResultSymbol(*expression.leftOperandSymbol());
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
    type::Type baseType = expression.getBase()->getType();
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
        expression.setResultSymbol(addrSym);
        // Keep expression type as the array for sizeof / isArray() checks.
        expression.setType(memberTy);
    } else {
        expression.setResultSymbol(symbolTable.createTemporarySymbol(memberTy));
        expression.setFieldAddressSymbol(
                symbolTable.createTemporarySymbol(type::pointer(memberTy)));
    }
}

} // namespace semantic_analyzer
