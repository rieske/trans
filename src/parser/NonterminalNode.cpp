#include "NonterminalNode.h"

#include <iterator>
#include <sstream>

#include "../semantic_analyzer/AbstractSyntaxTree.h"


namespace parser {

NonterminalNode::NonterminalNode(string l, vector<ParseTreeNode *> children, SymbolTable *st,
		unsigned lineNumber) :
		ParseTreeNode(l, children),
		s_table(st),
		sourceLine(lineNumber) {
}

NonterminalNode::NonterminalNode(string l, vector<ParseTreeNode *> children) :
		NonterminalNode(l, children, nullptr, 0) {
}

string NonterminalNode::getAttr() const {
	return attr;
}

std::ostringstream &NonterminalNode::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "\t";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << ">" << std::endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "\t";
	oss << "</" << elabel << ">" << std::endl;
	return oss;
}

void NonterminalNode::semanticError(std::string description) {
	error = true;
	std::cerr << semantic_analyzer::AbstractSyntaxTree::getFileName() << ":" << sourceLine << ": error: " << description;
}

}
