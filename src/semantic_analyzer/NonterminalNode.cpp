#include "NonterminalNode.h"

#include <iostream>

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeVisitor.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

NonterminalNode::NonterminalNode(std::string typeId) :
		label { typeId } {
}

NonterminalNode::NonterminalNode(string label, vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned lineNumber) :
		label { label },
		s_table(st),
		sourceLine(lineNumber) {
	assign_children(children);
}

NonterminalNode::NonterminalNode(string l, vector<AbstractSyntaxTreeNode *> children) :
		NonterminalNode(l, children, nullptr, 0) {
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

std::string NonterminalNode::typeId() const {
	return label;
}

void NonterminalNode::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
