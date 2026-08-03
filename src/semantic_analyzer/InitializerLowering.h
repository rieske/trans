#ifndef SEMANTIC_INITIALIZERLOWERING_H_
#define SEMANTIC_INITIALIZERLOWERING_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AggregateInitSink.h"
#include "symbols/AnnotationStore.h"
#include "symbols/GlobalInitializer.h"
#include "types/Type.h"

namespace ast {
class Expression;
class InitializerListExpression;
}

namespace semantic_analyzer {

class SymbolTable;

// Brace-list length for T[] completion. Error if a designator cannot fold.
struct IncompleteArrayBound {
    enum class Kind { None, Bound, Error };
    Kind kind { Kind::None };
    int bound { 0 };
    std::string error;

    static IncompleteArrayBound none() {
        return {};
    }
    static IncompleteArrayBound sized(int n) {
        IncompleteArrayBound r;
        r.kind = Kind::Bound;
        r.bound = n;
        return r;
    }
    static IncompleteArrayBound fail(std::string message) {
        IncompleteArrayBound r;
        r.kind = Kind::Error;
        r.error = std::move(message);
        return r;
    }
};

IncompleteArrayBound incompleteArrayBoundFromInitializer(const ast::Expression* init);

// Complete T[] from a (already visited) string or brace initializer.
// Returns an error message when a designated index cannot fold.
std::optional<std::string> completeIncompleteArrayFromInitializer(type::Type& type,
        ast::Expression* init);

// Expand brace/designated/string initializers into field stores at byte offsets.
// Empty when initializer is null or the sink reported an error (already diagnosed).
std::vector<symbols::FieldInit> lowerToFieldInits(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        AggregateInitHost& host);

// Flatten file-scope / static aggregate init into multi-word .data operands.
// Declined = not a handled shape (caller may try other folds / generic error).
// Failed  = shape handled; sink already reported (do not emit a second generic).
// Ok      = words filled.
enum class DataWordsLowering { Declined, Failed, Ok };

// Shared init-shape: feedInitializer, lowerToDataWords, DeclarationAnalyzer, walk.
enum class ObjectInitKind { None, CharArrayString, AggregateBrace, Scalar };

ObjectInitKind classifyObjectInit(const type::Type& type, ast::Expression* init);

DataWordsLowering lowerToDataWords(type::Type objectType,
        ast::Expression* initializer,
        AggregateInitHost& host,
        type::Type& outObjectType,
        std::vector<symbols::DataWord>& outWords);

} // namespace semantic_analyzer

#endif
