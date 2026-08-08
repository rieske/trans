#include "SemanticAnalysisVisitorInternal.h"

#include "AggregateDesignatorPath.h"
#include "AggregateInitSinks.h"
#include "AggregateInitWalk.h"

#include "ast/ConstantExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/StringLiteralDecode.h"

#include <limits>

namespace semantic_analyzer {

namespace {

bool isCharacterElement(const type::Type& elementType) {
    type::Type e = elementType.withoutTopLevelQualifiers();
    return e.isPrimitive() && e.getSize() == 1;
}

} // namespace

std::optional<int> incompleteArrayBoundFromInitializer(ast::Expression* init, const type::Type& elementType,
        std::string& error) {
    error.clear();
    if (!init) {
        return std::nullopt;
    }
    if (auto* literal = dynamic_cast<ast::StringLiteralExpression*>(init)) {
        if (!isCharacterElement(elementType)) {
            return std::nullopt;
        }
        const int n = util::stringLiteralArrayLength(literal->getValue());
        return n > 0 ? std::optional<int> { n } : std::nullopt;
    }
    auto* list = dynamic_cast<ast::InitializerListExpression*>(init);
    if (!list) {
        return std::nullopt;
    }
    const auto& elements = list->getElements();
    if (elements.empty()) {
        return std::nullopt;
    }
    if (elements.size() == 1 && !elements.front().isDesignated() && elements.front().value) {
        if (auto* nested = dynamic_cast<ast::StringLiteralExpression*>(elements.front().value.get())) {
            if (isCharacterElement(elementType)) {
                const int n = util::stringLiteralArrayLength(nested->getValue());
                if (n > 0) {
                    return n;
                }
            }
        }
    }
    int next = 0;
    int bound = 0;
    for (const auto& el : elements) {
        int idx = next;
        if (el.isDesignated()) {
            std::vector<ast::DesignatorStep> steps;
            if (!foldDesignatorSteps(el, steps, error) || steps.empty()) {
                if (error.empty()) {
                    error = "empty designator";
                }
                return std::nullopt;
            }
            if (steps.front().kind != ast::DesignatorStep::Kind::Index) {
                error = "designated initializer member not found";
                return std::nullopt;
            }
            if (!steps.front().index) {
                error = "designated array index is not a constant expression";
                return std::nullopt;
            }
            const long v = *steps.front().index;
            if (v < 0 || v > static_cast<long>(std::numeric_limits<int>::max())) {
                error = "designated initializer index out of range";
                return std::nullopt;
            }
            idx = static_cast<int>(v);
        }
        next = idx + 1;
        if (next > bound) {
            bound = next;
        }
    }
    return bound > 0 ? std::optional<int> { bound } : std::nullopt;
}

namespace {

bool trySetFloatingGlobalConstant(SymbolTable& symbolTable, const std::string& name,
        ast::Expression* expr) {
    auto* constant = dynamic_cast<ast::ConstantExpression*>(expr);
    if (!constant) {
        return false;
    }
    std::string immediate;
    if (!util::floatingLiteralImmediate(constant->getValue(), immediate)) {
        return false;
    }
    symbolTable.setGlobalMultiWordInitializer(name, { std::move(immediate) });
    return true;
}

} // namespace

void SemanticAnalysisVisitor::lowerLocalInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& objectType) {
    if (!declarator.hasInitializer()) {
        return;
    }

    if (symbolTable.isAtFileScope()) {
        long initValue = 0;
        if (type::isFloating(objectType)
                && trySetFloatingGlobalConstant(symbolTable, declarator.getName(),
                        declarator.getInitializer())) {
            return;
        }
        if (declarator.getInitializer()->evaluateConstant(initValue)) {
            symbolTable.setGlobalInitializer(declarator.getName(), initValue);
            return;
        }
        if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
            if (!(objectType.isRecord() || objectType.isArray())) {
                if (list->getElements().size() == 1 && list->getElements().front().value) {
                    auto* value = list->getElements().front().value.get();
                    if (type::isFloating(objectType)
                            && trySetFloatingGlobalConstant(symbolTable, declarator.getName(),
                                    value)) {
                        return;
                    }
                    if (value->evaluateConstant(initValue)) {
                        symbolTable.setGlobalInitializer(declarator.getName(), initValue);
                        return;
                    }
                }
                semanticError("global brace initializer is not a constant expression", declarator.getContext());
                return;
            }
            const int wordCount = type::object_abi::dataWords(objectType.getSize());
            if (wordCount <= 0) {
                return;
            }
            std::vector<std::string> words(static_cast<std::size_t>(wordCount), "0");
            DataWordSink sink { *this, declarator.getContext(), words, wordCount };
            walkAggregateInit(objectType, list, 0, sink);
            if (!sink.ok()) {
                return;
            }
            symbolTable.setGlobalMultiWordInitializer(declarator.getName(), std::move(words));
            return;
        }
        semanticError("global initializer is not a constant expression", declarator.getContext());
        return;
    }

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isRecord() || objectType.isArray()) {
            std::vector<symbols::StructFieldInit> plan;
            FieldPlanSink sink { *this, symbolTable, annotations(), declarator.getContext(), plan };
            walkAggregateInit(objectType, list, 0, sink);
            if (sink.ok()) {
                annotations().setStructFieldInits(&declarator, std::move(plan));
            }
            return;
        }
        if (list->getElements().size() > 1) {
            semanticError("excess elements in scalar initializer", declarator.getContext());
            return;
        }
        if (list->getElements().empty() || !list->getElements().front().value) {
            return;
        }
        ast::Expression* value = list->getElements().front().value.get();
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
        while (nested) {
            if (nested->getElements().size() > 1) {
                semanticError("excess elements in scalar initializer", declarator.getContext());
                return;
            }
            if (nested->getElements().empty() || nested->getElements().front().value == nullptr) {
                return;
            }
            value = nested->getElements().front().value.get();
            nested = dynamic_cast<ast::InitializerListExpression*>(value);
        }
        if (value && value->hasResultSymbol(annotations())) {
            type::Type src = assignSourceType(*value, objectType, annotations());
            if (!value->holdsAggregateAddress() || objectType.isPointer()) {
                typeCheck(src, objectType, declarator.getContext());
            }
            list->setResultSymbol(annotations(), *value->getResultSymbol(annotations()));
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
