#ifndef SEMANTICSYNTAXTREEBUILDER_H_
#define SEMANTICSYNTAXTREEBUILDER_H_

#include <string>
#include <vector>

#include "../parser/GrammarSymbol.h"
#include "../parser/ParseTreeBuilder.h"
#include "param_decl_node.h"

class Token;

class SemanticTreeBuilder: public ParseTreeBuilder {
public:
	SemanticTreeBuilder();
	virtual ~SemanticTreeBuilder();

	void makeTerminalNode(const Token& token) override;
	void makeNonterminalNode(std::string definingSymbol, Production production) override;

	SyntaxTree build() override;

private:
	void adjustScope(std::string lexeme);

	int currentLine { 0 };
	SymbolTable* symbolTable;
	SymbolTable* currentScope;

	vector<ParamDeclNode *> declaredParams;

	bool containsSemanticErrors = false;
};

#endif /* SEMANTICSYNTAXTREEBUILDER_H_ */
