#include "NonterminalNode.h"

#include <iostream>
#include <stdexcept>


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
}

NonterminalNode::NonterminalNode(string l, vector<AbstractSyntaxTreeNode *> children) :
		NonterminalNode(l, children, nullptr, 0) {
}

void NonterminalNode::semanticError(std::string description) {
	error = true;
	std::cerr /*<< semantic_analyzer::AbstractSyntaxTree::getFileName()*/<< ":" << sourceLine << ": error: " << description;
}

bool NonterminalNode::getErrorFlag() const {
	return error;
}

vector<Quadruple *> NonterminalNode::getCode() const {
	return code;
}

std::string NonterminalNode::typeId() const {
	return label;
}

void NonterminalNode::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	throw std::runtime_error {"NonterminalNode::accept"};
}

}
