#ifndef _CONTEXTUAL_SYNTAX_NODE_BUILDER_
#define _CONTEXTUAL_SYNTAX_NODE_BUILDER_

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"
#include "parser/Grammar.h"

namespace ast {

class ContextualSyntaxNodeBuilder {
public:
    ContextualSyntaxNodeBuilder(const parser::Grammar& grammar);
    ~ContextualSyntaxNodeBuilder();

    void updateContext(
            int definingSymbol,
            const std::vector<int>& production,
            AbstractSyntaxTreeBuilderContext& context) const;

private:
    void noCreatorDefined(int definingSymbol, const std::vector<int>& production) const;

    static void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);

    std::unordered_map<int, std::map<std::vector<int>, std::function<void(AbstractSyntaxTreeBuilderContext&)>>>nodeCreatorRegistry;

    const parser::Grammar* grammar;
};

} // namespace ast

#endif // _CONTEXTUAL_SYNTAX_NODE_BUILDER_
