#include "NonterminalNode.h"

#include "AbstractSyntaxTree.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

NonterminalNode::NonterminalNode(string l, vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned lineNumber) :
		s_table(st),
		sourceLine(lineNumber) {
	assign_label(l);
	assign_children(children);
}

NonterminalNode::NonterminalNode(string l, vector<AbstractSyntaxTreeNode *> children) :
		NonterminalNode(l, children, nullptr, 0) {
}

string NonterminalNode::getValue() const {
	return attr;
}

void NonterminalNode::semanticError(std::string description) {
	error = true;
	std::cerr << semantic_analyzer::AbstractSyntaxTree::getFileName() << ":" << sourceLine << ": error: " << description;
}

bool NonterminalNode::getErrorFlag() const {
	return error;
}

void NonterminalNode::assign_label(string &l) {
	if (l.size() > 1) {
		if (l.at(l.size() - 1) == '>')
			l = l.substr(0, l.size() - 1);
		if (l.at(0) == '<')
			l = l.substr(1, l.size() - 1);

		label = l;
	} else if (l.size() == 1) {
		label = l;
	} else {
		label = "undefined";
	}
}

void NonterminalNode::assign_children(vector<AbstractSyntaxTreeNode *> children) {
	subtrees.insert(subtrees.end(), children.begin(), children.end());
}

vector<Quadruple *> NonterminalNode::getCode() const {
	return code;
}

}
