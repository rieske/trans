#ifndef AGGREGATE_DESIGNATOR_PATH_H_
#define AGGREGATE_DESIGNATOR_PATH_H_

#include "ast/InitializerListExpression.h"
#include "types/Type.h"

#include <string>
#include <vector>

namespace semantic_analyzer {

// Path from root aggregate to a designated (or resume) subobject.
struct DesignatorPathItem {
    bool isArray { false };
    int index { 0 };
};

// Fold index expressions into constant designator steps. On failure, error is set.
bool foldDesignatorSteps(const ast::InitializerElement& el,
        std::vector<ast::DesignatorStep>& stepsOut, std::string& error);

// Resolve designator steps to a place and a path from root for resume after fill.
bool resolveDesignator(const type::Type& destType, int baseOffset,
        const std::vector<ast::DesignatorStep>& steps, type::FoundMember& outSlot,
        std::vector<DesignatorPathItem>& path, std::string& error);

// Advance path past the current designated object to the next current-object slot.
// Unions do not resume into sibling arms.
bool advanceDesignatorPath(std::vector<DesignatorPathItem>& path, const type::Type& root);

} // namespace semantic_analyzer

#endif
