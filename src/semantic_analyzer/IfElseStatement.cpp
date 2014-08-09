#include "IfElseStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

IfElseStatement::IfElseStatement(std::unique_ptr<Expression> testExpression,
        std::unique_ptr<AbstractSyntaxTreeNode> truthyBody, std::unique_ptr<AbstractSyntaxTreeNode> falsyBody,
        SymbolTable *st) :
        NonterminalNode(st, 0),
        testExpression { std::move(testExpression) },
        truthyBody { std::move(truthyBody) },
        falsyBody { std::move(falsyBody) } {
    code = this->testExpression->getCode();
    vector<Quadruple *> matched_code = this->truthyBody->getCode();
    vector<Quadruple *> unmatched_code = this->falsyBody->getCode();

    SymbolEntry *else_label = s_table->newLabel();
    SymbolEntry *exit_label = s_table->newLabel();

    matched_code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
    matched_code.push_back(new Quadruple(LABEL, else_label, NULL, NULL));

    unmatched_code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));

    SymbolEntry *expr_val = this->testExpression->getPlace();
    code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
    code.push_back(new Quadruple(JE, else_label, NULL, NULL));

    code.insert(code.end(), matched_code.begin(), matched_code.end());
    code.insert(code.end(), unmatched_code.begin(), unmatched_code.end());
}

IfElseStatement::~IfElseStatement() {
}

void IfElseStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

const SymbolEntry* IfElseStatement::getFalsyLabel() const {
    return falsyLabel;
}

void IfElseStatement::setFalsyLabel(SymbolEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

const SymbolEntry* IfElseStatement::getTruthyLabel() const {
    return truthyLabel;
}

void IfElseStatement::setTruthyLabel(SymbolEntry* truthyLabel) {
    this->truthyLabel = truthyLabel;
}

} /* namespace semantic_analyzer */

