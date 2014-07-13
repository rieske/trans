#include "Pointer.h"

namespace semantic_analyzer {

const std::string Pointer::ID { "<ptr>" };

Pointer::Pointer(Pointer* pointer, ParseTreeNode* dereferenceOperator) :
		NonterminalNode(ID, { pointer, dereferenceOperator }) {
	type = pointer->getType();
	type = "p" + type;
}

Pointer::Pointer(ParseTreeNode* dereferenceOperator) :
		NonterminalNode(ID, { dereferenceOperator }) {
	type = "p";
}

ostringstream &Pointer::asXml(ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " type=\"" << type << "\" " << ">" << endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">" << endl;
	return oss;
}

string Pointer::getType() const {
	return type;
}

}
