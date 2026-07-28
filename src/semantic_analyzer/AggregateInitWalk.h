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

} // namespace semantic_analyzer

#endif
