#include "VariableDeclaration.h"

#include <iterator>
#include <sstream>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "DeclarationList.h"
#include "Expression.h"
#include "TerminalSymbol.h"

using parser::NonterminalNode;

namespace semantic_analyzer {

const std::string VariableDeclaration::ID { "<var_decl>" };

VariableDeclaration::VariableDeclaration(TerminalSymbol typeSpecifier, DeclarationList* declarationList, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { declarationList }, st, ln) {
	basic_type = typeSpecifier.value;
	decls = declarationList->getDecls();
	int errLine;
	for (vector<Declaration *>::const_iterator it = decls.begin(); it != decls.end(); it++) {
		if (basic_type == "void" && (*it)->getType() == "") {
			semanticError("error: variable or field ‘" + (*it)->getName() + "’ declared void\n");
		} else if (0 != (errLine = s_table->insert((*it)->getName(), basic_type, (*it)->getType(), sourceLine))) {
			std::ostringstream errorDescription;
			errorDescription << "symbol " << (*it)->getName() << " declaration conflicts with previous declaration on line " << errLine
					<< "\n";
			semanticError(errorDescription.str());
		}
	}
}

VariableDeclaration::VariableDeclaration(TerminalSymbol typeSpecifier, DeclarationList* declarationList, Expression* assignmentExpression,
		SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { declarationList, assignmentExpression }, st, ln) {
	basic_type = typeSpecifier.value;
	decls = declarationList->getDecls();
	int errLine;
	for (vector<Declaration *>::const_iterator it = decls.begin(); it != decls.end(); it++) {
		if (basic_type == "void" && (*it)->getType() == "") {
			semanticError("error: variable or field ‘" + (*it)->getName() + "’ declared void\n");
			return;
		} else if (0 != (errLine = s_table->insert((*it)->getName(), basic_type, (*it)->getType(), sourceLine))) {
			std::ostringstream errorDescription;
			errorDescription << "symbol " << (*it)->getName() << " declaration conflicts with previous declaration on line " << errLine
					<< "\n";
			semanticError(errorDescription.str());
			return;
		}
	}
	vector<Quadruple *> a_code = assignmentExpression->getCode();
	code.insert(code.end(), a_code.begin(), a_code.end());
	code.push_back(new Quadruple(ASSIGN, assignmentExpression->getPlace(),
	NULL, s_table->lookup(decls[decls.size() - 1]->getName())));
}

std::ostringstream &VariableDeclaration::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " basic_type=\"" << xmlEncode(basic_type) << "\"";
	for (unsigned i = 0; i < decls.size(); i++)
		decls.at(i)->output_attr(oss, i);
	if (init_val != "")
		oss << " init_val=\"" << init_val << "\"";
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

}