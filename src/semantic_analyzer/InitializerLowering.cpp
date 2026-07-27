#include "SemanticAnalysisVisitorInternal.h"

#include <functional>
#include <memory>

#include "ast/InitializerListExpression.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::lowerLocalInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& objectType) {
    if (!declarator.hasInitializer()) {
        return;
    }

    if (symbolTable.isAtFileScope()) {
        long initValue = 0;
        if (declarator.getInitializer()->evaluateConstant(initValue)) {
            symbolTable.setGlobalInitializer(declarator.getName(), initValue);
            return;
        }
        if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
            if (list->getElements().size() == 1 && list->getElements().front()
                    && list->getElements().front()->evaluateConstant(initValue)) {
                symbolTable.setGlobalInitializer(declarator.getName(), initValue);
                return;
            }
            semanticError("global brace initializer is not a constant expression", declarator.getContext());
            return;
        }
        semanticError("global initializer is not a constant expression", declarator.getContext());
        return;
    }

    using Plan = std::vector<symbols::StructFieldInit>;
    std::function<void(Plan&, const type::Type&, int, ast::Expression*)> lowerMember;
    std::function<void(Plan&, const type::Type&, int, ast::InitializerListExpression*)> lowerBrace;

    std::function<ast::Expression*(ast::Expression*)> scalarFromValue;
    scalarFromValue = [&](ast::Expression* value) -> ast::Expression* {
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
        if (!nested) {
            return value;
        }
        if (nested->getElements().size() > 1) {
            semanticError("excess elements in scalar initializer", declarator.getContext());
            return nullptr;
        }
        if (nested->getElements().empty() || !nested->getElements().front()) {
            return nullptr;
        }
        return scalarFromValue(nested->getElements().front().get());
    };

    auto emitWholeUnionZero = [&](Plan& plan, int offsetBytes, const type::Type& unionType) {
        symbols::StructFieldInit field;
        field.offsetBytes = offsetBytes;
        auto addr = symbolTable.createTemporarySymbol(type::pointer(unionType));
        field.addressName = addr.getName();
        auto zero = symbolTable.createTemporarySymbol(unionType);
        field.zeroInitialize = true;
        field.sourceName = zero.getName();
        plan.push_back(std::move(field));
    };

    lowerMember = [&](Plan& plan, const type::Type& memberType, int offsetBytes, ast::Expression* value) {
        if (memberType.isUnion()) {
            auto* nestedList = dynamic_cast<ast::InitializerListExpression*>(value);
            if (nestedList) {
                lowerBrace(plan, memberType, offsetBytes, nestedList);
                return;
            }
            if (value) {
                if (memberType.memberCount() < 1) {
                    return;
                }
                std::string name;
                type::Type first = type::voidType();
                int off = 0;
                if (!memberType.memberAt(0, name, first, off)) {
                    return;
                }
                lowerMember(plan, first, offsetBytes + off, value);
                return;
            }
            // Missing value: zero whole union object (full size), not first arm only.
            emitWholeUnionZero(plan, offsetBytes, memberType);
            return;
        }

        if (memberType.isStructure() || memberType.isArray()) {
            auto* nestedList = dynamic_cast<ast::InitializerListExpression*>(value);
            if (nestedList) {
                lowerBrace(plan, memberType, offsetBytes, nestedList);
                return;
            }
            if (value) {
                semanticError(
                        "aggregate member initializer requires nested braces (not implemented)",
                        declarator.getContext());
                return;
            }
            if (memberType.isStructure()) {
                for (int i = 0; i < memberType.memberCount(); ++i) {
                    std::string name;
                    type::Type mt = type::voidType();
                    int off = 0;
                    if (!memberType.memberAt(i, name, mt, off)) {
                        break;
                    }
                    lowerMember(plan, mt, offsetBytes + off, nullptr);
                }
                return;
            }
            const int n = memberType.getArraySize();
            if (n <= 0) {
                semanticError("array brace initializers for incomplete arrays are not implemented",
                        declarator.getContext());
                return;
            }
            const int stride = memberType.getElementStride();
            const type::Type elem = memberType.getElementType();
            for (int i = 0; i < n; ++i) {
                lowerMember(plan, elem, offsetBytes + i * stride, nullptr);
            }
            return;
        }

        ast::Expression* scalar = scalarFromValue(value);
        symbols::StructFieldInit field;
        field.offsetBytes = offsetBytes;
        auto addr = symbolTable.createTemporarySymbol(type::pointer(memberType));
        field.addressName = addr.getName();
        if (scalar && scalar->hasResultSymbol(annotations())) {
            typeCheck(assignSourceType(*scalar, memberType, annotations()), memberType, declarator.getContext());
            field.zeroInitialize = false;
            field.sourceName = scalar->getResultSymbol(annotations())->getName();
        } else {
            auto zero = symbolTable.createTemporarySymbol(memberType);
            field.zeroInitialize = true;
            field.sourceName = zero.getName();
        }
        plan.push_back(std::move(field));
    };

    lowerBrace = [&](Plan& plan, const type::Type& destType, int baseOffset, ast::InitializerListExpression* list) {
        const auto& elements = list->getElements();
        if (destType.isUnion()) {
            if (static_cast<int>(elements.size()) > 1) {
                semanticError("excess elements in union initializer", declarator.getContext());
            }
            if (elements.empty() || !elements.front()) {
                emitWholeUnionZero(plan, baseOffset, destType);
                return;
            }
            if (destType.memberCount() < 1) {
                return;
            }
            std::string name;
            type::Type first = type::voidType();
            int off = 0;
            if (!destType.memberAt(0, name, first, off)) {
                return;
            }
            // First-member init only (value or nested braces for aggregate first arm).
            lowerMember(plan, first, baseOffset + off, elements.front().get());
            return;
        }
        if (destType.isStructure()) {
            if (static_cast<int>(elements.size()) > destType.memberCount()) {
                semanticError("excess elements in structure initializer", declarator.getContext());
            }
            for (int i = 0; i < destType.memberCount(); ++i) {
                std::string name;
                type::Type memberType = type::voidType();
                int offset = 0;
                if (!destType.memberAt(i, name, memberType, offset)) {
                    break;
                }
                ast::Expression* value = nullptr;
                if (i < static_cast<int>(elements.size()) && elements[static_cast<std::size_t>(i)]) {
                    value = elements[static_cast<std::size_t>(i)].get();
                }
                lowerMember(plan, memberType, baseOffset + offset, value);
            }
            return;
        }
        if (destType.isArray()) {
            const int n = destType.getArraySize();
            if (n <= 0) {
                semanticError("array brace initializers for incomplete arrays are not implemented",
                        declarator.getContext());
                return;
            }
            if (static_cast<int>(elements.size()) > n) {
                semanticError("excess elements in array initializer", declarator.getContext());
            }
            const int stride = destType.getElementStride();
            const type::Type elem = destType.getElementType();
            for (int i = 0; i < n; ++i) {
                ast::Expression* value = nullptr;
                if (i < static_cast<int>(elements.size()) && elements[static_cast<std::size_t>(i)]) {
                    value = elements[static_cast<std::size_t>(i)].get();
                }
                lowerMember(plan, elem, baseOffset + i * stride, value);
            }
            return;
        }
        semanticError("brace initializer for non-aggregate type", declarator.getContext());
    };

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isRecord() || objectType.isArray()) {
            Plan plan;
            lowerBrace(plan, objectType, 0, list);
            annotations().setStructFieldInits(&declarator, std::move(plan));
            return;
        }
        // Top-level scalar braces: share scalarFromValue (nested excess, unwrap).
        if (list->getElements().size() > 1) {
            semanticError("excess elements in scalar initializer", declarator.getContext());
            return;
        }
        if (list->getElements().empty() || !list->getElements().front()) {
            return;
        }
        ast::Expression* scalar = scalarFromValue(list->getElements().front().get());
        if (scalar && scalar->hasResultSymbol(annotations())) {
            type::Type src = assignSourceType(*scalar, objectType, annotations());
            if (!scalar->holdsAggregateAddress() || objectType.isPointer()) {
                typeCheck(src, objectType, declarator.getContext());
            }
            // Reuse single-element list assign path: set result on list so CG can assign.
            list->setResultSymbol(annotations(), *scalar->getResultSymbol(annotations()));
        }
        return;
    }

    ast::Expression* initExpr = declarator.getInitializer();
    if (initExpr && initExpr->hasResultSymbol(annotations())) {
        type::Type src = assignSourceType(*initExpr, objectType, annotations());
        if (!initExpr->holdsAggregateAddress() || objectType.isPointer()) {
            typeCheck(src, objectType, declarator.getContext());
        }
    }
}

} // namespace semantic_analyzer
