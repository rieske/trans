#ifndef _ABSTRACT_SYNTAX_TREE_BUILDER_H_
#define _ABSTRACT_SYNTAX_TREE_BUILDER_H_

#include <stddef.h>
#include <memory>
#include <string>

#include "../parser/Production.h"
#include "../parser/SyntaxTreeBuilder.h"
#include "AbstractSyntaxTreeBuilderContext.h"
#include "ContextualSyntaxNodeBuilder.h"

namespace semantic_analyzer {

class AbstractSyntaxTreeBuilder: public parser::SyntaxTreeBuilder {
public:
	AbstractSyntaxTreeBuilder();
	virtual ~AbstractSyntaxTreeBuilder();

	void makeTerminalNode(std::string type, std::string value, size_t line) override;
	void makeNonterminalNode(std::string definingSymbol, parser::Production production) override;

	std::unique_ptr<parser::SyntaxTree> build() override;

private:
	ContextualSyntaxNodeBuilder syntaxNodeBuilder;
	AbstractSyntaxTreeBuilderContext context;
};

}

#endif /* _ABSTRACT_SYNTAX_TREE_BUILDER_H_ */
