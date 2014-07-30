#include "LoopHeader.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Expression.h"

namespace semantic_analyzer {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(SymbolTable* symbolTable) :
		NonterminalNode(symbolTable, 0) {
}

vector<Quadruple *> LoopHeader::getBPList() const {
	return backpatchList;
}

SymbolEntry *LoopHeader::getLoopLabel() const {
	return loop_label;
}

vector<Quadruple *> LoopHeader::getAfterLoop() const {
	return afterLoop;
}

}
