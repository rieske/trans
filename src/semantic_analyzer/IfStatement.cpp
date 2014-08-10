#include "IfStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body,
        SymbolTable *st) :
        NonterminalNode(st, 0),
        testExpression { std::move(testExpression) },
        body { std::move(body) } {
    code = this->testExpression->getCode();
    vector<Quadruple *> stmt_code = this->body->getCode();
    SymbolEntry *exit_label = s_table->newLabel();
    stmt_code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
    SymbolEntry *expr_val = this->testExpression->getPlace();
    code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
    code.push_back(new Quadruple(JE, exit_label, NULL, NULL));
    code.insert(code.end(), stmt_code.begin(), stmt_code.end());
}

IfStatement::~IfStatement() {
}

void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::setFalsyLabel(SymbolEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

SymbolEntry* IfStatement::getFalsyLabel() const {
    return falsyLabel;
}

} /* namespace semantic_analyzer */
