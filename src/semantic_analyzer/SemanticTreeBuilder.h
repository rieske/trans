#ifndef SEMANTICSYNTAXTREEBUILDER_H_
#define SEMANTICSYNTAXTREEBUILDER_H_

#include <stddef.h>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "../parser/Production.h"
#include "../parser/SyntaxTreeBuilder.h"
#include "NonterminalNodeFactory.h"
#include "ParameterDeclaration.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class SemanticTreeBuilder: public parser::SyntaxTreeBuilder {
public:
	SemanticTreeBuilder();
	virtual ~SemanticTreeBuilder();

	void makeTerminalNode(std::string type, std::string value, size_t line) override;
	void makeNonterminalNode(std::string definingSymbol, parser::Production production) override;

	std::unique_ptr<parser::SyntaxTree> build() override;

private:
	void adjustScope(std::string lexeme);

	void noSemanticActionsFoundFor(std::string definingSymbol, const parser::Production& production) const;

	std::vector<parser::ParseTreeNode*> getChildrenForReduction(int childrenCount);

	int currentLine { 0 };
	SymbolTable* symbolTable;
	SymbolTable* currentScope;

	vector<ParameterDeclaration *> declaredParams;

	bool containsSemanticErrors = false;

	NonterminalNodeFactory nonterminalNodeFactory;

	std::stack<parser::ParseTreeNode *> syntaxStack;

	std::stack<TerminalSymbol> terminalSymbols;
};

}

#endif /* SEMANTICSYNTAXTREEBUILDER_H_ */
