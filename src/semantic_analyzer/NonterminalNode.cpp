#include "NonterminalNode.h"

#include <iostream>

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeVisitor.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

NonterminalNode::NonterminalNode(string label, vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned lineNumber) :
		label{label},
		s_table(st),
		sourceLine(lineNumber) {
	assign_children(children);
}

NonterminalNode::NonterminalNode(string l, vector<AbstractSyntaxTreeNode *> children) :
		NonterminalNode(l, children, nullptr, 0) {
}

std::string NonterminalNode::getType() const {
	return label;
}

string NonterminalNode::getValue() const {
	return attr;
}

std::vector<AbstractSyntaxTreeNode*> NonterminalNode::getChildren() const {
	return subtrees;
}

void NonterminalNode::semanticError(std::string description) {
	error = true;
	std::cerr << semantic_analyzer::AbstractSyntaxTree::getFileName() << ":" << sourceLine << ": error: " << description;
}

bool NonterminalNode::getErrorFlag() const {
	return error;
}

void NonterminalNode::assign_children(vector<AbstractSyntaxTreeNode *> children) {
	subtrees.insert(subtrees.end(), children.begin(), children.end());
}

vector<Quadruple *> NonterminalNode::getCode() const {
	return code;
}

void NonterminalNode::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
