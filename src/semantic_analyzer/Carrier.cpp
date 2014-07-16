#include "Carrier.h"

#include <sstream>

namespace semantic_analyzer {

Carrier::Carrier(string label, vector<ParseTreeNode *> &children) :
		NonterminalNode(label, children) {
	if (children.size() == 1) {
		attr = children.at(0)->getAttr();
	}
	for (const auto child : children) {
		vector<Quadruple *> c_code = child->getCode();
		code.insert(code.end(), c_code.begin(), c_code.end());
	}
}

std::ostringstream &Carrier::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " attribute=\"" << xmlEncode(attr) << "\"" << " code_size=\"" << code.size() << "\" " << ">\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

}
