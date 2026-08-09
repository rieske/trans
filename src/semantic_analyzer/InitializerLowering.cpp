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

ast::StringLiteralExpression* charArrayStringLiteral(ast::Expression* init) {
    if (auto* literal = dynamic_cast<ast::StringLiteralExpression*>(init)) {
        return literal;
    }
    auto* list = dynamic_cast<ast::InitializerListExpression*>(init);
    if (!list || list->getElements().size() != 1 || list->getElements().front().isDesignated()
            || !list->getElements().front().value) {
        return nullptr;
    }
    return dynamic_cast<ast::StringLiteralExpression*>(list->getElements().front().value.get());
}

std::unique_ptr<ast::InitializerListExpression> braceListFromStringBytes(
        const std::vector<unsigned char>& bytes, const translation_unit::Context& context) {
    std::vector<ast::InitializerElement> elements;
    elements.reserve(bytes.size());
    for (unsigned char b : bytes) {
        ast::Constant constant { std::to_string(static_cast<int>(b)), type::signedInteger(), context };
        elements.emplace_back(std::make_unique<ast::ConstantExpression>(std::move(constant)));
    }
    return std::make_unique<ast::InitializerListExpression>(std::move(elements));
}

} // namespace

bool SemanticAnalysisVisitor::rewriteCharArrayStringInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& type) {
    if (!declarator.hasInitializer() || !type.isArray() || !isCharacterElement(type.getElementType())) {
        return true;
    }
    auto* literal = charArrayStringLiteral(declarator.getInitializer());
    if (!literal) {
        return true;
    }
    std::vector<unsigned char> bytes = util::decodeStringLiteralBytes(literal->getValue());
    if (!type.isIncompleteArray()) {
        const int n = type.getArraySize();
        if (n > 0 && static_cast<int>(bytes.size()) > n) {
            if (static_cast<int>(bytes.size()) - 1 > n) {
                semanticError("excess elements in array initializer", declarator.getContext());
                return false;
            }
            bytes.resize(static_cast<std::size_t>(n));
        }
    }
    declarator.setInitializer(braceListFromStringBytes(bytes, declarator.getContext()));
    return true;
}

IncompleteArrayBound incompleteArrayBoundFromInitializer(ast::Expression* init) {
    if (!init) {
        return IncompleteArrayBound::none();
    }
    auto* list = dynamic_cast<ast::InitializerListExpression*>(init);
    if (!list) {
        return IncompleteArrayBound::none();
    }
    const auto& elements = list->getElements();
    if (elements.empty()) {
        return IncompleteArrayBound::none();
    }
    int next = 0;
    int bound = 0;
    for (const auto& el : elements) {
        int idx = next;
        if (el.isDesignated()) {
            std::vector<ast::DesignatorStep> steps;
            std::string error;
            if (!foldDesignatorSteps(el, steps, error) || steps.empty()) {
                return IncompleteArrayBound::fail(error.empty() ? "empty designator" : std::move(error));
            }
            if (steps.front().kind != ast::DesignatorStep::Kind::Index) {
                return IncompleteArrayBound::fail("designated initializer member not found");
            }
            if (!steps.front().index) {
                return IncompleteArrayBound::fail("designated array index is not a constant expression");
            }
            const long v = *steps.front().index;
            if (v < 0 || v > static_cast<long>(std::numeric_limits<int>::max())) {
                return IncompleteArrayBound::fail("designated initializer index out of range");
            }
            idx = static_cast<int>(v);
        }
        next = idx + 1;
        if (next > bound) {
            bound = next;
        }
    }
    return bound > 0 ? IncompleteArrayBound::sized(bound) : IncompleteArrayBound::none();
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
    symbolTable.setMultiWordInitializer(name, { std::move(immediate) });
    return true;
}

} // namespace

void SemanticAnalysisVisitor::lowerAggregateList(ast::InitializedDeclarator& declarator,
        const type::Type& objectType, const ast::InitializerListExpression* list) {
    auto* holder = declarator.getHolder(annotations());
    if (holder && holder->isGlobal()) {
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
        symbolTable.setMultiWordInitializer(declarator.getName(), std::move(words));
        return;
    }
    std::vector<symbols::StructFieldInit> plan;
    FieldPlanSink sink { *this, symbolTable, annotations(), declarator.getContext(), plan };
    walkAggregateInit(objectType, list, 0, sink);
    if (sink.ok()) {
        annotations().setStructFieldInits(&declarator, std::move(plan));
    }
}

void SemanticAnalysisVisitor::lowerLocalInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& objectType) {
    if (!declarator.hasInitializer()) {
        return;
    }

    auto* holder = declarator.getHolder(annotations());
    if (holder && holder->isGlobal()) {
        long initValue = 0;
        if (type::isFloating(objectType)
                && trySetFloatingGlobalConstant(symbolTable, declarator.getName(),
                        declarator.getInitializer())) {
            return;
        }
        if (declarator.getInitializer()->evaluateConstant(initValue)) {
            symbolTable.setConstantInitializer(declarator.getName(), initValue);
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
                        symbolTable.setConstantInitializer(declarator.getName(), initValue);
                        return;
                    }
                }
                semanticError("global brace initializer is not a constant expression", declarator.getContext());
                return;
            }
            lowerAggregateList(declarator, objectType, list);
            return;
        }
        semanticError("global initializer is not a constant expression", declarator.getContext());
        return;
    }

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isRecord() || objectType.isArray()) {
            lowerAggregateList(declarator, objectType, list);
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
