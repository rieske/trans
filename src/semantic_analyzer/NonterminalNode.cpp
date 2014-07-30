#include "NonterminalNode.h"

#include <stdexcept>

namespace semantic_analyzer {

NonterminalNode::NonterminalNode(SymbolTable *st, unsigned lineNumber) :
		s_table(st),
		sourceLine(lineNumber) {
}

void NonterminalNode::semanticError(std::string description) {
	throw std::runtime_error { "semantic error at line:" + std::to_string(sourceLine) + ": error: " + description };
}

vector<Quadruple *> NonterminalNode::getCode() const {
	return code;
}

}
