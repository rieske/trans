#ifndef SEMANTICSYNTAXTREEBUILDER_H_
#define SEMANTICSYNTAXTREEBUILDER_H_

#include <string>
#include <vector>

#include "../parser/ParseTreeBuilder.h"
#include "../parser/Production.h"
#include "NonterminalNodeFactory.h"
#include "ParameterDeclaration.h"

namespace semantic_analyzer {

class SemanticTreeBuilder: public parser::ParseTreeBuilder {
public:
	SemanticTreeBuilder();
	virtual ~SemanticTreeBuilder();

	void makeTerminalNode(const Token& token) override;
	void makeNonterminalNode(std::string definingSymbol, parser::Production production) override;

	parser::SyntaxTree build() override;

private:
	void adjustScope(std::string lexeme);

	void noSemanticActionsFoundFor(std::string definingSymbol, const parser::Production& production) const;

	int currentLine { 0 };
	SymbolTable* symbolTable;
	SymbolTable* currentScope;

	vector<ParameterDeclaration *> declaredParams;

	bool containsSemanticErrors = false;

	NonterminalNodeFactory nonterminalNodeFactory;

	//std::stack<AExpressionsNode*> assignmentExpressionLists;
	//std::stack<AExprNode*> assignmentExpressions;
};

}

#endif /* SEMANTICSYNTAXTREEBUILDER_H_ */
