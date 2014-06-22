#ifndef SEMANTICSYNTAXTREEBUILDER_H_
#define SEMANTICSYNTAXTREEBUILDER_H_

#include <string>
#include <vector>

#include "param_decl_node.h"
#include "ParseTreeBuilder.h"

class Token;

class SemanticTreeBuilder: public ParseTreeBuilder {
public:
	SemanticTreeBuilder();
	virtual ~SemanticTreeBuilder();

	void makeTerminalNode(const Token& token) override;
	void makeNonterminalNode(std::string definingSymbol, Production production) override;

private:
	void adjustScope(std::string lexeme);

	int currentLine { 0 };
	SymbolTable* currentScope;

	vector<ParamDeclNode *> declaredParams;
};

#endif /* SEMANTICSYNTAXTREEBUILDER_H_ */
