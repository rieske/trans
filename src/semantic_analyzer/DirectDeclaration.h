#ifndef _DIR_DECL_NODE_H_
#define _DIR_DECL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "ParameterDeclaration.h"

namespace semantic_analyzer {

class LogicalOrExpression;
class Declaration;
class ParameterList;
class TerminalSymbol;

class DirectDeclaration: public parser::NonterminalNode {
public:
	DirectDeclaration(Declaration* declaration, SymbolTable *st, unsigned ln);
	DirectDeclaration(TerminalSymbol identifier, SymbolTable *st, unsigned ln);
	DirectDeclaration(DirectDeclaration* directDeclaration, ParameterList* parameterList, SymbolTable *st, unsigned ln);
	DirectDeclaration(DirectDeclaration* directDeclaration, LogicalOrExpression* logicalExpression, SymbolTable *st, unsigned ln);
	DirectDeclaration(DirectDeclaration* directDeclaration, SymbolTable *st, unsigned ln);

	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;

	string getName() const;
	string getType() const;

	vector<ParameterDeclaration *> getParams() const;

	static const std::string ID;

private:
	string name;
	string type;

	vector<ParameterDeclaration *> params;
};

}

#endif // _DIR_DECL_NODE_H_