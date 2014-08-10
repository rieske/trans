#include "LoopStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "LoopHeader.h"

namespace semantic_analyzer {

LoopStatement::LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<AbstractSyntaxTreeNode> body,
        SymbolTable *st) :
        NonterminalNode(st, 0),
        header { std::move(header) },
        body { std::move(body) } {
    code = this->header->getCode();
    vector<Quadruple *> more_code = this->body->getCode();
    SymbolEntry *loop_label = this->header->getLoopLabel();
    SymbolEntry *exit_label = s_table->newLabel();

    // backpatch
    vector<Quadruple *> bpList = this->header->getBPList();
    for (unsigned i = 0; i < bpList.size(); i++) {
        bpList[0]->setArg1(exit_label);
    }

    code.insert(code.end(), more_code.begin(), more_code.end());

    vector<Quadruple *> afterLoop = this->header->getAfterLoop();
    code.insert(code.end(), afterLoop.begin(), afterLoop.end());
    code.push_back(new Quadruple(GOTO, loop_label, NULL, NULL));
    code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
}

LoopStatement::~LoopStatement() {
}

void LoopStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
