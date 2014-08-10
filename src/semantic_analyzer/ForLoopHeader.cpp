#include "ForLoopHeader.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

ForLoopHeader::ForLoopHeader(std::unique_ptr<Expression> initialization, std::unique_ptr<Expression> clause,
        std::unique_ptr<Expression> increment, SymbolTable *st) :
        LoopHeader(st, std::move(increment)),
        initialization { std::move(initialization) },
        clause { std::move(clause) } {
    code = this->initialization->getCode();    // šitas vieną kartą įvykdomas
    vector<Quadruple *> expr_code = this->clause->getCode();    // čia tikrinimas pradžioj
    SymbolEntry *expr_val = this->clause->getPlace();    // tikrinama reikšmė
    afterLoop = this->increment->getCode();    // šitas po kūno vykdomas

    loop_label = s_table->newLabel();
    code.push_back(new Quadruple(LABEL, loop_label, NULL, NULL));
    code.insert(code.end(), expr_code.begin(), expr_code.end());
    code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
    Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
    code.push_back(bp);
    backpatchList.push_back(bp);
}

ForLoopHeader::~ForLoopHeader() {
}

void ForLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
