#include "Declaration.h"

#include <iterator>
#include <vector>
#include <sstream>

#include "DirectDeclaration.h"
#include "Pointer.h"

using parser::NonterminalNode;

namespace semantic_analyzer {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(Pointer* pointer, DirectDeclaration* directDeclaration) :
		NonterminalNode(ID, { pointer, directDeclaration }) {
	type = pointer->getType();
	name = directDeclaration->getName();
	type = type + directDeclaration->getType();
}

std::ostringstream &Declaration::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " name=\"" << xmlEncode(name) << "\"";
	if (type != "")
		oss << " type=\"" << type << "\"";
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

void Declaration::output_attr(std::ostringstream &oss, unsigned nr) const {
	oss << " name" << nr << "=\"" << name << "\"";
	if (type != "")
		oss << " type" << nr << "=\"" << type << "\"";
}

string Declaration::getName() const {
	return name;
}

string Declaration::getType() const {
	return type;
}

}
