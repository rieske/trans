#ifndef _CONTEXTUAL_SYNTAX_NODE_BUILDER_
#define _CONTEXTUAL_SYNTAX_NODE_BUILDER_

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"

namespace ast {

class ContextualSyntaxNodeBuilder {
public:
    ContextualSyntaxNodeBuilder();
    ~ContextualSyntaxNodeBuilder();

    void updateContext(
            std::string definingSymbol,
            const std::vector<std::string>& production,
            AbstractSyntaxTreeBuilderContext& context) const;

private:
    void noCreatorDefined(std::string definingSymbol, const std::vector<std::string>& production) const;

    static void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);

    std::unordered_map<std::string, std::map<std::vector<std::string>, std::function<void(AbstractSyntaxTreeBuilderContext&)>>>nodeCreatorRegistry;
};

} // namespace ast

#endif // _CONTEXTUAL_SYNTAX_NODE_BUILDER_
