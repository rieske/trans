#include "Pointer.h"

namespace semantic_analyzer {

const std::string Pointer::ID { "<ptr>" };

Pointer::Pointer(Pointer* pointer) :
		NonterminalNode(ID, { pointer }) {
	type = pointer->getType();
	type = "p" + type;
}

Pointer::Pointer() :
		NonterminalNode(ID, { }) {
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
