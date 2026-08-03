#ifndef AGGREGATE_INIT_WALK_H_
#define AGGREGATE_INIT_WALK_H_

#include "AggregateInitSink.h"
#include "types/Type.h"

namespace ast {
class InitializerListExpression;
}

namespace semantic_analyzer {

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink);

// Place one expression (designator value, scalar peel, or single-value elision).
// holesAlreadyZero: parent walk already issued a full-object zero for this storage.
void placeInitValue(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink, bool holesAlreadyZero = false);

} // namespace semantic_analyzer

#endif
