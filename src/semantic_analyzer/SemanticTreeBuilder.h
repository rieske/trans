#ifndef SEMANTICSYNTAXTREEBUILDER_H_
#define SEMANTICSYNTAXTREEBUILDER_H_

#include <stddef.h>
#include <memory>
#include <string>

#include "../parser/Production.h"
#include "../parser/SyntaxTreeBuilder.h"
#include "AbstractSyntaxTreeBuilderContext.h"
#include "SyntaxNodeFactory.h"

namespace semantic_analyzer {

class SemanticTreeBuilder: public parser::SyntaxTreeBuilder {
public:
	SemanticTreeBuilder();
	virtual ~SemanticTreeBuilder();

	void makeTerminalNode(std::string type, std::string value, size_t line) override;
	void makeNonterminalNode(std::string definingSymbol, parser::Production production) override;

	std::unique_ptr<parser::SyntaxTree> build() override;

private:
	SyntaxNodeFactory syntaxNodeFactory;
	AbstractSyntaxTreeBuilderContext context;
};

}

#endif /* SEMANTICSYNTAXTREEBUILDER_H_ */
