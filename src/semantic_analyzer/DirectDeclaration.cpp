#include "DirectDeclaration.h"

#include <cstdlib>
#include <iterator>
#include <sstream>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "ParameterList.h"
#include "Declaration.h"
#include "LogicalOrExpression.h"

namespace semantic_analyzer {

const std::string DirectDeclaration::ID { "<dir_decl>" };

DirectDeclaration::DirectDeclaration(ParseTreeNode* openParenthesis, Declaration* declaration, ParseTreeNode* closeParenthesis, SymbolTable *st,
		unsigned ln) :
		NonterminalNode(ID, { openParenthesis, declaration, closeParenthesis }, st, ln) {
	//if (reduction == "'(' <decl> ')'") {  // XXX: čia žiūrėti reiks

	//}
}

DirectDeclaration::DirectDeclaration(ParseTreeNode* identifier, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { identifier }, st, ln) {
	name = identifier->getAttr();
}

DirectDeclaration::DirectDeclaration(DirectDeclaration* directDeclaration, ParseTreeNode* openParenthesis, ParameterList* parameterList,
		ParseTreeNode* closeParenthesis, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { directDeclaration, openParenthesis, parameterList, closeParenthesis }, st, ln) {
	name = directDeclaration->getName();
	type = "f";
	params = parameterList->getParams();
	int errLine;
	if (0 != (errLine = s_table->insert(name, "", type, sourceLine))) {
		std::ostringstream errorDescription;
		errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
		semanticError(errorDescription.str());
	} else {
		SymbolEntry *place = s_table->lookup(name);
		for (unsigned i = 0; i < params.size(); i++) {
			if (params[i]->getPlace() == NULL)
				exit(2);
			SymbolEntry *entry = params[i]->getPlace();
			place->setParam(entry);
		}
	}
}

DirectDeclaration::DirectDeclaration(DirectDeclaration* directDeclaration, ParseTreeNode* openBrace, LogicalOrExpression* logicalExpression,
		ParseTreeNode* closeBrace, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { directDeclaration, openBrace, logicalExpression, closeBrace }, st, ln) {
	// FIXME: not implemented
	name = directDeclaration->getName();
	type = "a";
}

DirectDeclaration::DirectDeclaration(DirectDeclaration* directDeclaration, ParseTreeNode* openParenthesis, ParseTreeNode* closeParenthesis, SymbolTable *st,
		unsigned ln) :
		NonterminalNode(ID, { directDeclaration, openParenthesis, closeParenthesis }, st, ln) {
	name = directDeclaration->getName();
	type = "f";
	int errLine;
	if (0 != (errLine = s_table->insert(name, "", type, sourceLine))) {
		std::ostringstream errorDescription;
		errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
		semanticError(errorDescription.str());
	}
}

ostringstream &DirectDeclaration::asXml(ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " name=\"" << xmlEncode(name) << "\" ";
	if (type != "")
		oss << "type=\"" << type << "\" ";
	if (params.size())
		oss << "params=\"true\" ";
	oss << ">" << endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">" << endl;
	return oss;
}

string DirectDeclaration::getName() const {
	return name;
}

string DirectDeclaration::getType() const {
	return type;
}

vector<ParameterDeclaration *> DirectDeclaration::getParams() const {
	return params;
}

}
