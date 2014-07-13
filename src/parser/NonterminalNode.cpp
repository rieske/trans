#include "NonterminalNode.h"

#include <iterator>

#include "SyntaxTree.h"

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

ostringstream &NonterminalNode::asXml(ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << ">" << endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">" << endl;
	return oss;
}

void NonterminalNode::semanticError(std::string description) {
	error = true;
	cerr << parser::SyntaxTree::getFileName() << ":" << sourceLine << ": error: " << description;
}

