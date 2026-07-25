#include "SemanticAnalysisVisitorInternal.h"

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

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isStructure()) {
            if (static_cast<int>(list->getElements().size()) > objectType.memberCount()) {
                semanticError("excess elements in structure initializer", declarator.getContext());
            }
            std::vector<symbols::StructFieldInit> plan;
            plan.reserve(static_cast<std::size_t>(objectType.memberCount()));
            for (int i = 0; i < objectType.memberCount(); ++i) {
                std::string name;
                type::Type memberType = type::voidType();
                int offset = 0;
                if (!objectType.memberAt(i, name, memberType, offset)) {
                    break;
                }
                if (memberType.isStructure() || memberType.isArray()) {
                    semanticError(
                            "aggregate member initializer requires nested braces (not implemented)",
                            declarator.getContext());
                }

                symbols::StructFieldInit field;
                field.offsetBytes = offset;
                auto addr = symbolTable.createTemporarySymbol(type::pointer(memberType));
                field.addressName = addr.getName();

                const bool hasElement = i < static_cast<int>(list->getElements().size())
                        && list->getElements()[static_cast<std::size_t>(i)]
                        && list->getElements()[static_cast<std::size_t>(i)]->hasResultSymbol();
                if (hasElement) {
                    auto& element = list->getElements()[static_cast<std::size_t>(i)];
                    typeCheck(assignSourceType(*element, memberType), memberType, declarator.getContext());
                    field.zeroInitialize = false;
                    field.sourceName = element->getResultSymbol()->getName();
                } else {
                    auto zero = symbolTable.createTemporarySymbol(memberType);
                    field.zeroInitialize = true;
                    field.sourceName = zero.getName();
                }
                plan.push_back(std::move(field));
            }
            annotations().setStructFieldInits(&declarator, std::move(plan));
            return;
        }
        if (objectType.isArray()) {
            semanticError("array brace initializers are not implemented", declarator.getContext());
            return;
        }
        if (list->getElements().size() > 1) {
            semanticError("excess elements in scalar initializer", declarator.getContext());
        }
    }

    ast::Expression* initExpr = declarator.getInitializer();
    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(initExpr);
            list && list->getElements().size() == 1) {
        initExpr = list->getElements().front().get();
    }
    if (initExpr && initExpr->hasResultSymbol()) {
        type::Type src = assignSourceType(*initExpr, objectType);
        if (!initExpr->holdsAggregateAddress() || objectType.isPointer()) {
            typeCheck(src, objectType, declarator.getContext());
        }
    }
}

} // namespace semantic_analyzer
