#include "InitializerLowering.h"

#include "AggregateDesignatorPath.h"
#include "AggregateInitSinks.h"
#include "AggregateInitWalk.h"
#include "SymbolTable.h"

#include "ast/ConstantExpression.h"
#include "ast/EffectiveInitializer.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/StringLiteralDecode.h"

#include <limits>

namespace semantic_analyzer {

namespace {

bool isCharacterElement(const type::Type& elementType) {
    return type::isCharacter(elementType.withoutTopLevelQualifiers());
}

} // namespace

ast::StringLiteralExpression* asCharArrayStringLiteral(ast::Expression* init) {
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

namespace {

type::Type completeArrayTypeFromString(const type::Type& arrayType,
        const ast::StringLiteralExpression* strLit) {
    if (!arrayType.isIncompleteArray() || !strLit) {
        return arrayType;
    }
    const int length = util::stringLiteralArrayLength(strLit->getValue());
    return type::array(arrayType.getElementType(), length);
}

} // namespace

IncompleteArrayBound incompleteArrayBoundFromInitializer(const ast::Expression* init) {
    if (!init) {
        return IncompleteArrayBound::none();
    }
    const auto* list = dynamic_cast<const ast::InitializerListExpression*>(init);
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
                return IncompleteArrayBound::fail(
                        error.empty() ? "empty designator" : std::move(error));
            }
            if (steps.front().kind != ast::DesignatorStep::Kind::Index) {
                return IncompleteArrayBound::fail("designated initializer member not found");
            }
            if (!steps.front().index) {
                return IncompleteArrayBound::fail(
                        "designated array index is not a constant expression");
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

std::optional<std::string> completeIncompleteArrayFromInitializer(type::Type& type,
        ast::Expression* init) {
    if (!type.isIncompleteArray() || !init) {
        return std::nullopt;
    }
    if (auto* strLit = asCharArrayStringLiteral(init)) {
        if (isCharacterElement(type.getElementType())) {
            type = completeArrayTypeFromString(type, strLit);
            return std::nullopt;
        }
        // Non-char T[] = { "..." }: one brace element, not a string-length complete.
    }
    const IncompleteArrayBound bound = incompleteArrayBoundFromInitializer(init);
    if (bound.kind == IncompleteArrayBound::Kind::Error) {
        return bound.error;
    }
    if (bound.kind == IncompleteArrayBound::Kind::Bound) {
        type = type::array(type.getElementType(), bound.bound);
    }
    return std::nullopt;
}

namespace {

bool isCharArrayString(const type::Type& objectType, ast::Expression* initializer) {
    return asCharArrayStringLiteral(initializer) && isCharArrayType(objectType);
}

} // namespace

ObjectInitKind classifyObjectInit(const type::Type& type, ast::Expression* init) {
    if (!init) {
        return ObjectInitKind::None;
    }
    if (isCharArrayString(type, init)) {
        return ObjectInitKind::CharArrayString;
    }
    if (dynamic_cast<ast::InitializerListExpression*>(init) && type.isAggregate()) {
        return ObjectInitKind::AggregateBrace;
    }
    return ObjectInitKind::Scalar;
}

namespace {

void feedInitializer(const type::Type& objectType, ast::Expression* initializer,
        AggregateInitSink& sink) {
    switch (classifyObjectInit(objectType, initializer)) {
    case ObjectInitKind::CharArrayString:
        if (!sink.placeStringArray(0, objectType, initializer)) {
            sink.error("string literal initializer requires character array");
        }
        return;
    case ObjectInitKind::AggregateBrace:
        walkAggregateInit(objectType, dynamic_cast<ast::InitializerListExpression*>(initializer),
                0, sink);
        return;
    case ObjectInitKind::Scalar: {
        std::string excess;
        ast::Expression* peeled = ast::effectiveInitializer(objectType, initializer, &excess);
        if (!excess.empty()) {
            sink.error(excess);
        }
        if (peeled) {
            placeInitValue(objectType, 0, peeled, sink);
        }
        return;
    }
    case ObjectInitKind::None:
        return;
    }
}

} // namespace

type::Type lowerToFieldInits(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        AggregateInitHost& host,
        FieldInitSink sink) {
    if (!initializer) {
        return objectType;
    }
    std::vector<symbols::StructFieldInit> plan;
    FieldPlanSink fieldSink { host, symbolTable, initializer->getContext(), plan };
    feedInitializer(objectType, initializer, fieldSink);
    if (fieldSink.ok()) {
        for (auto& field : plan) {
            sink(std::move(field));
        }
    }
    return objectType;
}

DataWordsLowering lowerToDataWords(type::Type objectType,
        ast::Expression* initializer,
        AggregateInitHost& host,
        type::Type& outObjectType,
        std::vector<symbols::DataWord>& outWords) {
    if (!initializer || !objectType.isAggregate()) {
        return DataWordsLowering::Declined;
    }
    outObjectType = objectType;
    const int wordCount = type::object_abi::dataWords(objectType.getSize());
    if (wordCount <= 0) {
        outWords.clear();
        return DataWordsLowering::Ok;
    }
    outWords.assign(static_cast<std::size_t>(wordCount), symbols::ConstantInit { 0 });
    DataWordSink sink { host, initializer->getContext(), outWords, wordCount };
    const ObjectInitKind kind = classifyObjectInit(objectType, initializer);
    if (kind != ObjectInitKind::CharArrayString && kind != ObjectInitKind::AggregateBrace) {
        return DataWordsLowering::Declined;
    }
    feedInitializer(objectType, initializer, sink);
    return sink.ok() ? DataWordsLowering::Ok : DataWordsLowering::Failed;
}

} // namespace semantic_analyzer
