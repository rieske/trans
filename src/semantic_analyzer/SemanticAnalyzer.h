#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <string>

#include "../parser/Production.h"

namespace parser {
class SyntaxTree;
class SyntaxTreeBuilder;
} /* namespace parser */

class SemanticAnalyzer {
public:
	SemanticAnalyzer(parser::SyntaxTreeBuilder* builder);
	virtual ~SemanticAnalyzer();

	std::unique_ptr<parser::SyntaxTree> getSyntaxTree();

	void makeTerminalNode(std::string type, std::string value, size_t line);
	void makeNonterminalNode(std::string definingSymbol, parser::Production production);

	void syntaxError();

private:
	std::unique_ptr<parser::SyntaxTreeBuilder> builder;
};

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
