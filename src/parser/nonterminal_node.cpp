#include "nonterminal_node.h"

#include <iterator>

#include "SyntaxTree.h"

NonterminalNode::NonterminalNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned lineNumber) :
		ParseTreeNode(l, children),
		reduction(productionStr(production)),
		s_table(st),
		sourceLine(lineNumber) {
}

NonterminalNode::NonterminalNode(string l, vector<ParseTreeNode *> &children, Production production) :
		NonterminalNode(l, children, production, nullptr, 0) {
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
	cerr << SyntaxTree::getFileName() << ":" << sourceLine << ": error: " << description;
}

std::string NonterminalNode::productionStr(const Production& production) const {
	std::string ret = "";
	for (auto& symbol : production) {
		if (symbol->getSymbol().length() && *symbol->getSymbol().begin() != '<' && *symbol->getSymbol().end() - 1 != '>') {
			ret += "'" + symbol->getSymbol() + "'";
		} else {
			ret += symbol->getSymbol();
		}
		ret += " ";
	}
	return ret.substr(0, ret.size() - 1);
}
