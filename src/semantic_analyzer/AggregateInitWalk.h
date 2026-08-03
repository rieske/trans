#ifndef AGGREGATE_INIT_WALK_H_
#define AGGREGATE_INIT_WALK_H_

#include "AggregateInitSink.h"
#include "types/Type.h"

namespace ast {
class Expression;
class InitializerListExpression;
}

namespace semantic_analyzer {

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink);

// Place one expression (scalar peel, designator value, or first-slot elision).
void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink);

} // namespace semantic_analyzer

#endif
