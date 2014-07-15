#include "DeclarationList.h"

#include <iterator>

namespace semantic_analyzer {

const std::string DeclarationList::ID { "<decls>" };

DeclarationList::DeclarationList(DeclarationList* declarationList, Declaration* declaration) :
		NonterminalNode(ID, { declarationList, declaration }) {
	decls = declarationList->getDecls();
	decls.push_back(declaration);
}

DeclarationList::DeclarationList(Declaration* declaration) :
		NonterminalNode(ID, { declaration }) {
	decls.push_back(declaration);
}

ostringstream &DeclarationList::asXml(ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel;
	outputDecls(oss);
	oss << " >" << endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">" << endl;
	return oss;
}

void DeclarationList::outputDecls(ostringstream &oss) const {
	for (unsigned i = 0; i < decls.size(); i++)
		decls.at(i)->output_attr(oss, i);
}

vector<Declaration *> DeclarationList::getDecls() const {
	return decls;
}

}
