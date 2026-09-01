#ifndef _CONTEXTUAL_SYNTAX_NODE_BUILDER_
#define _CONTEXTUAL_SYNTAX_NODE_BUILDER_

#include <functional>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"
#include "parser/Grammar.h"

namespace ast {

class ContextualSyntaxNodeBuilder {
public:
    ContextualSyntaxNodeBuilder(const parser::Grammar& grammar);
    ~ContextualSyntaxNodeBuilder();

    void updateContext(const parser::Production& production, AbstractSyntaxTreeBuilderContext& context) const;

private:
    using Creator = std::function<void(AbstractSyntaxTreeBuilderContext&)>;

    void bind(int lhs, std::vector<int> rhs, Creator creator);
    void noCreatorDefined(const parser::Production& production,
            AbstractSyntaxTreeBuilderContext& context) const;

    static void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);

    std::vector<Creator> creators_;

    const parser::Grammar* grammar;
};

} // namespace ast

#endif // _CONTEXTUAL_SYNTAX_NODE_BUILDER_
