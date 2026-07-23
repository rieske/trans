#ifndef SEMANTIC_INITIALIZERLOWERING_H_
#define SEMANTIC_INITIALIZERLOWERING_H_

#include <functional>
#include <string>
#include <vector>

#include "ast/InitializedDeclarator.h"
#include "types/Type.h"

namespace ast {
class Expression;
class InitializerListExpression;
class StringLiteralExpression;
}

namespace semantic_analyzer {

class SymbolTable;

// Sink for per-field stores produced by brace/designated/string local init.
using FieldInitSink = std::function<void(ast::StructFieldInit)>;

// Complete incomplete array type from brace list element count / designators.
type::Type completeArrayTypeFromList(const type::Type& arrayType,
        const ast::InitializerListExpression* list);

// Complete incomplete array type from string literal (including trailing NUL).
type::Type completeArrayTypeFromString(const type::Type& arrayType,
        const ast::StringLiteralExpression* strLit);

// Expand brace/designated/string initializers into field stores at byte offsets.
// Used for local aggregates and compound literals.
// Completes incomplete array types; returns the (possibly updated) object type.
type::Type lowerToFieldInits(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        FieldInitSink sink);

// Flatten file-scope / static aggregate init into multi-word .data operands.
// Returns false if initializer is not a brace list for an aggregate.
// Completes incomplete arrays into outObjectType.
bool lowerToDataWords(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        type::Type& outObjectType,
        std::vector<std::string>& outWords);

} // namespace semantic_analyzer

#endif // SEMANTIC_INITIALIZERLOWERING_H_
