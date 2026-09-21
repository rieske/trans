#ifndef _CONTEXTUAL_SYNTAX_NODE_BUILDER_
#define _CONTEXTUAL_SYNTAX_NODE_BUILDER_

#include <cstddef>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"
#include "parser/Grammar.h"

namespace ast {

class ContextualSyntaxNodeBuilder {
public:
    ContextualSyntaxNodeBuilder(const parser::Grammar& grammar);

    void updateContext(const parser::Production& production, AbstractSyntaxTreeBuilderContext& context) const;

private:
    using Creator = void (*)(AbstractSyntaxTreeBuilderContext&);

    void bind(int lhs, std::vector<int> rhs, Creator creator);
    void bindBoth(int matchedLhs, int unmatchedLhs, std::vector<int> rhs, Creator creator,
            std::size_t unmatchedIndex = static_cast<std::size_t>(-1));
    void noCreatorDefined(const parser::Production& production,
            AbstractSyntaxTreeBuilderContext& context) const;

    static void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);

    std::vector<Creator> creators_;

    const parser::Grammar* grammar;
};

} // namespace ast

#endif // _CONTEXTUAL_SYNTAX_NODE_BUILDER_
