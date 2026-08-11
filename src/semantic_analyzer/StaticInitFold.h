#ifndef SEMANTIC_ANALYZER_STATIC_INIT_FOLD_H_
#define SEMANTIC_ANALYZER_STATIC_INIT_FOLD_H_

#include <optional>

#include "symbols/StaticInit.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace ast {
class Expression;
}

namespace semantic_analyzer {

class SemanticAnalysisVisitor;

// Type-check, fold, convert to dest. Reports semantic errors. Nullopt means do not store.
std::optional<symbols::StaticInitValue> evaluateStaticInit(
        SemanticAnalysisVisitor& visitor, const ast::Expression& expr, const type::Type& dest,
        const translation_unit::Context& context);

} // namespace semantic_analyzer

#endif
