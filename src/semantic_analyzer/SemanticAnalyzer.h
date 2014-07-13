#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <string>

#include "../parser/Production.h"

class Token;

namespace parser {
class ParseTreeBuilder;
class SyntaxTree;
} /* namespace parser */


class SemanticAnalyzer {
public:
	SemanticAnalyzer(parser::ParseTreeBuilder* builder);
	virtual ~SemanticAnalyzer();

	parser::SyntaxTree getSyntaxTree();

	void makeTerminalNode(const Token& token);
	void makeNonterminalNode(std::string definingSymbol, parser::Production production);

	void syntaxError();

private:
	std::unique_ptr<parser::ParseTreeBuilder> builder;
};

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
