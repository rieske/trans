#ifndef _ABSTRACT_SYNTAX_TREE_BUILDER_H_
#define _ABSTRACT_SYNTAX_TREE_BUILDER_H_

#include <memory>
#include <string>

#include "parser/Grammar.h"
#include "parser/ParseTreeBuilder.h"
#include "parser/Production.h"
#include "parser/SyntaxTreeBuilder.h"
#include "AbstractSyntaxTreeBuilderContext.h"
#include "ContextualSyntaxNodeBuilder.h"

namespace ast {

class AbstractSyntaxTreeBuilder: public parser::SyntaxTreeBuilder {
public:
    AbstractSyntaxTreeBuilder(const parser::Grammar* grammar);
    virtual ~AbstractSyntaxTreeBuilder();

    void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) override;
    void makeNonterminalNode(int definingSymbol, parser::Production production) override;

    std::unique_ptr<parser::SyntaxTree> build() override;

private:
    ContextualSyntaxNodeBuilder syntaxNodeBuilder;
    AbstractSyntaxTreeBuilderContext treeBuilderContext;
};

}

#endif /* _ABSTRACT_SYNTAX_TREE_BUILDER_H_ */
