#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <string>

class ParseTreeBuilder;

class SyntaxTree;
class Token;

class SemanticAnalyzer {
public:
	SemanticAnalyzer(ParseTreeBuilder* builder);
	virtual ~SemanticAnalyzer();

	std::unique_ptr<SyntaxTree> build();
	void withSourceFileName(std::string fileName);

	void makeTerminalNode(const Token& token);
	void makeNonTerminalNode(std::string left, int childrenCount, std::string reduction);

	void syntaxError();

private:
	std::unique_ptr<ParseTreeBuilder> builder;
};

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
