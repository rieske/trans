#include "NonterminalNode.h"

#include <iostream>
#include <stdexcept>


using std::string;
using std::vector;

namespace semantic_analyzer {

NonterminalNode::NonterminalNode(std::string typeId) :
		label { typeId } {
}

NonterminalNode::NonterminalNode(SymbolTable *st, unsigned lineNumber) :
		s_table(st),
		sourceLine(lineNumber) {
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
