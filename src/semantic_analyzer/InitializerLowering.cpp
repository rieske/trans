#include "InitializerLowering.h"

#include "AggregateDesignatorPath.h"
#include "AggregateInitSinks.h"
#include "AggregateInitWalk.h"
#include "SymbolTable.h"

#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "types/ObjectAbi.h"
#include "util/StringLiteralDecode.h"

namespace semantic_analyzer {

namespace {

// Top-level incomplete array length. Uses foldDesignatorSteps so index fold
// matches the walk (delayed sizeof after SA visit of designator expressions).
int incompleteArrayLengthFromList(const ast::InitializerListExpression* list) {
    int length = 0;
    int positional = 0;
    for (const auto& element : list->getElements()) {
        if (!element.value) {
            continue;
        }
        if (element.isDesignated()) {
            std::vector<ast::DesignatorStep> steps;
            std::string err;
            if (!foldDesignatorSteps(element, steps, err)) {
                continue;
            }
            // Outermost incomplete T[]: only the first designator index sizes it.
            for (const auto& step : steps) {
                if (step.kind != ast::DesignatorStep::Kind::Index || !step.index) {
                    continue;
                }
                const int idx = static_cast<int>(*step.index);
                if (idx >= 0 && idx + 1 > length) {
                    length = idx + 1;
                }
                positional = idx + 1;
                break;
            }
        } else {
            if (positional + 1 > length) {
                length = positional + 1;
            }
            ++positional;
        }
    }
    return length;
}

} // namespace

type::Type completeArrayTypeFromList(const type::Type& arrayType,
        const ast::InitializerListExpression* list) {
    if (!arrayType.isArray() || arrayType.getArraySize() != 0 || !list) {
        return arrayType;
    }
    int length = incompleteArrayLengthFromList(list);
    if (length < 1) {
        length = 1;
    }
    return type::array(arrayType.getElementType(), length);
}

type::Type completeArrayTypeFromString(const type::Type& arrayType,
        const ast::StringLiteralExpression* strLit) {
    if (!arrayType.isArray() || arrayType.getArraySize() != 0 || !strLit) {
        return arrayType;
    }
    const int length = util::stringLiteralArrayLength(strLit->getValue());
    return type::array(arrayType.getElementType(), length);
}

type::Type lowerToFieldInits(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        AggregateInitHost& host,
        FieldInitSink sink) {
    if (!initializer) {
        return objectType;
    }
    // Same incomplete-array completion rule as lowerToDataWords.
    if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(initializer)) {
        if (objectType.isArray() && objectType.getArraySize() == 0) {
            objectType = completeArrayTypeFromString(objectType, strLit);
        }
    } else if (auto* list = dynamic_cast<ast::InitializerListExpression*>(initializer)) {
        if (objectType.isArray() && objectType.getArraySize() == 0) {
            objectType = completeArrayTypeFromList(objectType, list);
        }
    }
    std::vector<symbols::StructFieldInit> plan;
    FieldPlanSink fieldSink { host, symbolTable, initializer->getContext(), plan };
    if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(initializer)) {
        fieldSink.placeStringArray(0, objectType, strLit);
    } else if (auto* list = dynamic_cast<ast::InitializerListExpression*>(initializer);
            list && objectType.isAggregate()) {
        walkAggregateInit(objectType, list, 0, fieldSink);
    }
    if (fieldSink.ok()) {
        for (auto& field : plan) {
            sink(std::move(field));
        }
    }
    return objectType;
}

bool lowerToDataWords(type::Type objectType,
        ast::Expression* initializer,
        AggregateInitHost& host,
        type::Type& outObjectType,
        std::vector<std::string>& outWords) {
    auto* list = dynamic_cast<ast::InitializerListExpression*>(initializer);
    if (!list || !objectType.isAggregate()) {
        return false;
    }
    if (objectType.isArray() && objectType.getArraySize() == 0) {
        objectType = completeArrayTypeFromList(objectType, list);
    }
    outObjectType = objectType;
    const int wordCount = type::object_abi::dataWords(objectType.getSize());
    if (wordCount <= 0) {
        outWords.clear();
        return true;
    }
    outWords.assign(static_cast<std::size_t>(wordCount), "0");
    DataWordSink sink { host, initializer->getContext(), outWords, wordCount };
    walkAggregateInit(objectType, list, 0, sink);
    return sink.ok();
}

} // namespace semantic_analyzer
