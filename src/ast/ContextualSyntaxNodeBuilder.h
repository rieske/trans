#ifndef _CONTEXTUAL_SYNTAX_NODE_BUILDER_
#define _CONTEXTUAL_SYNTAX_NODE_BUILDER_

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"

namespace ast {

class AbstractSyntaxTreeBuilderContext;
class AbstractSyntaxTreeNode;

class ContextualSyntaxNodeBuilder {
public:
    ContextualSyntaxNodeBuilder();
    ~ContextualSyntaxNodeBuilder() = default;

    void updateContext(std::string definingSymbol, const std::vector<std::string>& production,
            AbstractSyntaxTreeBuilderContext& context) const;

private:
    static void noCreatorDefined(std::string definingSymbol, const std::vector<std::string>& production);

    static void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);

    static void whileLoopHeader(AbstractSyntaxTreeBuilderContext& context);
    static void forLoopHeader(AbstractSyntaxTreeBuilderContext& context);

    static void ifStatement(AbstractSyntaxTreeBuilderContext& context);
    static void loopStatement(AbstractSyntaxTreeBuilderContext& context);

    std::unordered_map<std::string, std::map<std::vector<std::string>, std::function<void(AbstractSyntaxTreeBuilderContext&)>>>nodeCreatorRegistry;
};

}
/* namespace ast */

#endif /* _CONTEXTUAL_SYNTAX_NODE_BUILDER_ */
