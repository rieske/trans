#include "SemanticAnalysisVisitorInternal.h"

#include "AggregateDesignatorPath.h"
#include "AggregateInitSinks.h"
#include "AggregateInitWalk.h"
#include "CharArrayStringInit.h"
#include "StaticInitFold.h"

#include "ast/ConstantExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/StringLiteralDecode.h"

#include <limits>
#include <optional>

namespace semantic_analyzer {

namespace {

const ast::StringLiteralExpression* charArrayStringLiteral(const ast::Expression* init) {
    if (auto* literal = dynamic_cast<const ast::StringLiteralExpression*>(init)) {
        return literal;
    }
    auto* list = dynamic_cast<const ast::InitializerListExpression*>(init);
    if (!list || list->getElements().size() != 1 || list->getElements().front().isDesignated()
            || !list->getElements().front().value) {
        return nullptr;
    }
    return dynamic_cast<const ast::StringLiteralExpression*>(list->getElements().front().value.get());
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

bool isCharArrayStringInit(const type::Type& destArray, const ast::Expression* value) {
    return value && destArray.isArray() && type::isCharacter(destArray.getElementType())
            && charArrayStringLiteral(value);
}

std::optional<std::vector<unsigned char>> charArrayBytesFromString(
        const type::Type& destArray, const ast::Expression* value, std::string* error) {
    if (!isCharArrayStringInit(destArray, value)) {
        return std::nullopt;
    }
    std::vector<unsigned char> bytes =
            util::decodeStringLiteralBytes(charArrayStringLiteral(value)->getValue());
    if (!destArray.isIncompleteArray()) {
        const int n = destArray.getArraySize();
        if (n > 0 && static_cast<int>(bytes.size()) > n) {
            if (static_cast<int>(bytes.size()) - 1 > n) {
                if (error) {
                    *error = "excess elements in array initializer";
                }
                return std::nullopt;
            }
            bytes.resize(static_cast<std::size_t>(n));
        }
    }
    return bytes;
}

bool SemanticAnalysisVisitor::rewriteCharArrayStringInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& type) {
    if (!declarator.hasInitializer()) {
        return true;
    }
    std::string err;
    auto bytes = charArrayBytesFromString(type, declarator.getInitializer(), &err);
    if (!err.empty()) {
        semanticError(err, declarator.getContext());
        return false;
    }
    if (bytes) {
        declarator.setInitializer(braceListFromStringBytes(*bytes, declarator.getContext()));
    }
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

// Global empty braces diagnose as non-constant; local empty braces are a no-op.
enum class EmptyScalarBrace { ErrorAsNonConstant, NoOp };

ast::Expression* unwrapScalarBrace(const ast::InitializerListExpression* list,
        SemanticAnalysisVisitor& visitor, const translation_unit::Context& context,
        EmptyScalarBrace emptyPolicy) {
    const auto& elements = list->getElements();
    if (elements.size() > 1) {
        visitor.semanticError("excess elements in scalar initializer", context);
        return nullptr;
    }
    if (elements.empty() || !elements.front().value) {
        if (emptyPolicy == EmptyScalarBrace::ErrorAsNonConstant) {
            visitor.semanticError("global initializer is not a constant expression", context);
        }
        return nullptr;
    }
    ast::Expression* value = elements.front().value.get();
    auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
    while (nested) {
        if (nested->getElements().size() > 1) {
            visitor.semanticError("excess elements in scalar initializer", context);
            return nullptr;
        }
        if (nested->getElements().empty() || !nested->getElements().front().value) {
            if (emptyPolicy == EmptyScalarBrace::ErrorAsNonConstant) {
                visitor.semanticError("global initializer is not a constant expression", context);
            }
            return nullptr;
        }
        value = nested->getElements().front().value.get();
        nested = dynamic_cast<ast::InitializerListExpression*>(value);
    }
    return value;
}

bool setStaticScalarInit(SemanticAnalysisVisitor& visitor, SymbolTable& symbolTable,
        const std::string& name, const type::Type& dest, ast::Expression* expr,
        const translation_unit::Context& context) {
    auto value = evaluateStaticInit(visitor, *expr, dest, context);
    if (!value) {
        return false;
    }
    symbolTable.setStaticInit(name, symbols::asDataWords(*value));
    return true;
}

} // namespace

bool SemanticAnalysisVisitor::applyIncompleteArrayBound(type::Type& type, ast::Expression* init,
        const translation_unit::Context& context) {
    if (!type.isIncompleteArray()) {
        return true;
    }
    const IncompleteArrayBound bound = incompleteArrayBoundFromInitializer(init);
    if (bound.kind == IncompleteArrayBound::Kind::Error) {
        semanticError(bound.error, context);
        return false;
    }
    if (bound.kind != IncompleteArrayBound::Kind::Bound) {
        return true;
    }
    try {
        type = type::array(type.getElementType(), bound.bound);
    } catch (const std::invalid_argument& ex) {
        semanticError(ex.what(), context);
        return false;
    }
    return true;
}

void SemanticAnalysisVisitor::planLocalAggregateFieldInits(symbols::NodeRef node,
        const type::Type& objectType, const ast::InitializerListExpression* list,
        const translation_unit::Context& context) {
    std::vector<symbols::StructFieldInit> plan;
    FieldPlanSink sink { *this, symbolTable, annotations(), context, plan };
    walkAggregateInit(objectType, list, 0, sink);
    if (sink.ok()) {
        annotations().setStructFieldInits(node, std::move(plan));
    }
}

void SemanticAnalysisVisitor::lowerLocalScalarBraceList(ast::InitializerListExpression& list,
        const type::Type& objectType, const translation_unit::Context& context) {
    ast::Expression* value = unwrapScalarBrace(&list, *this, context, EmptyScalarBrace::NoOp);
    if (!value || !value->hasResultSymbol(annotations())) {
        return;
    }
    decayArrayToPointer(*value, objectType, symbolTable, annotations());
    type::Type src = assignSourceType(*value, objectType, annotations());
    if (!value->holdsAggregateAddress() || objectType.isPointer()) {
        checkAssign(objectType, src, context, value);
    }
    list.setResultSymbol(annotations(), *value->getResultSymbol(annotations()));
    maybeSetConversion(&list, objectType, symbolTable, annotations());
}

void SemanticAnalysisVisitor::lowerAggregateList(ast::InitializedDeclarator& declarator,
        const type::Type& objectType, const ast::InitializerListExpression* list) {
    auto* holder = declarator.getHolder(annotations());
    if (holder && holder->isGlobal()) {
        const int wordCount = type::object_abi::dataWords(objectType.getSize());
        if (wordCount <= 0) {
            return;
        }
        std::vector<symbols::StaticInitValue> words(
                static_cast<std::size_t>(wordCount), symbols::StaticWord {});
        DataWordSink sink { *this, declarator.getContext(), words, wordCount };
        walkAggregateInit(objectType, list, 0, sink);
        if (!sink.ok()) {
            return;
        }
        symbolTable.setStaticInit(declarator.getName(), std::move(words));
        return;
    }
    planLocalAggregateFieldInits(&declarator, objectType, list, declarator.getContext());
}

void SemanticAnalysisVisitor::lowerLocalInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& objectType) {
    if (!declarator.hasInitializer()) {
        return;
    }

    auto* holder = declarator.getHolder(annotations());
    if (holder && holder->isGlobal()) {
        ast::Expression* init = declarator.getInitializer();
        auto* list = dynamic_cast<ast::InitializerListExpression*>(init);
        if (list && (objectType.isRecord() || objectType.isArray())) {
            lowerAggregateList(declarator, objectType, list);
            return;
        }
        ast::Expression* expr = init;
        if (list) {
            expr = unwrapScalarBrace(list, *this, declarator.getContext(),
                    EmptyScalarBrace::ErrorAsNonConstant);
            if (!expr) {
                return;
            }
        }
        setStaticScalarInit(*this, symbolTable, declarator.getName(), objectType, expr,
                declarator.getContext());
        return;
    }

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isRecord() || objectType.isArray()) {
            lowerAggregateList(declarator, objectType, list);
            return;
        }
        lowerLocalScalarBraceList(*list, objectType, declarator.getContext());
        return;
    }

    ast::Expression* initExpr = declarator.getInitializer();
    if (initExpr && initExpr->hasResultSymbol(annotations())) {
        decayArrayToPointer(*initExpr, objectType, symbolTable, annotations());
        type::Type src = assignSourceType(*initExpr, objectType, annotations());
        if (!initExpr->holdsAggregateAddress() || objectType.isPointer()) {
            checkAssign(objectType, src, declarator.getContext(), initExpr);
        }
        maybeSetConversion(initExpr, objectType, symbolTable, annotations());
    }
}

} // namespace semantic_analyzer
