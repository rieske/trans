#ifndef SEMANTIC_INITIALIZERLOWERING_H_
#define SEMANTIC_INITIALIZERLOWERING_H_

#include <functional>
#include <string>
#include <vector>

#include "AggregateInitSink.h"
#include "symbols/AnnotationStore.h"
#include "types/Type.h"

namespace ast {
class Expression;
class InitializerListExpression;
class StringLiteralExpression;
}

namespace semantic_analyzer {

class SymbolTable;

// Sink for per-field stores produced by brace/designated/string local init.
using FieldInitSink = std::function<void(symbols::StructFieldInit)>;

// Complete incomplete array type from brace list element count / designators.
type::Type completeArrayTypeFromList(const type::Type& arrayType,
        const ast::InitializerListExpression* list);

// Complete incomplete array type from string literal (including trailing NUL).
type::Type completeArrayTypeFromString(const type::Type& arrayType,
        const ast::StringLiteralExpression* strLit);

// Expand brace/designated/string initializers into field stores at byte offsets.
type::Type lowerToFieldInits(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        AggregateInitHost& host,
        FieldInitSink sink);

// Flatten file-scope / static aggregate init into multi-word .data operands.
bool lowerToDataWords(type::Type objectType,
        ast::Expression* initializer,
        AggregateInitHost& host,
        type::Type& outObjectType,
        std::vector<std::string>& outWords);

} // namespace semantic_analyzer

#endif
