#include "Declaration.h"

#include <iterator>
#include <vector>

#include "DirectDeclaration.h"
#include "Pointer.h"

namespace semantic_analyzer {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(Pointer* pointer, DirectDeclaration* directDeclaration) :
		NonterminalNode(ID, { pointer, directDeclaration }) {
	type = pointer->getType();
	name = directDeclaration->getName();
	type = type + directDeclaration->getType();
}

Declaration::Declaration(DirectDeclaration* directDeclaration) :
		NonterminalNode(ID, { directDeclaration }) {
	name = directDeclaration->getName();
	type = directDeclaration->getType();
}

ostringstream &Declaration::asXml(ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " name=\"" << xmlEncode(name) << "\"";
	if (type != "")
		oss << " type=\"" << type << "\"";
	oss << " >" << endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">" << endl;
	return oss;
}

void Declaration::output_attr(ostringstream &oss, unsigned nr) const {
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
