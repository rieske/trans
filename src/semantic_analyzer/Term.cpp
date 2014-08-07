#include "Term.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

const std::string Term::ID { "<term>" };

Term::Term(TerminalSymbol term, SymbolTable *st, unsigned ln) :
        Expression(st, ln),
        term { term } {

    if (term.type == "id") {
        if ( NULL != (resultPlace = s_table->lookup(term.value))) {
            value = "lval";
            basicType = resultPlace->getBasicType();
            extended_type = resultPlace->getExtendedType();
        } else {
            semanticError("symbol " + term.value + " is not defined\n");
        }
    } else if (term.type == "int_const") {
        value = "rval";
        basicType = BasicType::INTEGER;
        resultPlace = s_table->newTemp(basicType, "");
        code.push_back(new Quadruple(term.value, resultPlace));
    } else if (term.type == "float_const") {
        value = "rval";
        basicType = BasicType::FLOAT;
        resultPlace = s_table->newTemp(basicType, "");
        code.push_back(new Quadruple(term.value, resultPlace));
    } else if (term.type == "literal") {
        value = "rval";
        basicType = BasicType::CHARACTER;
        resultPlace = s_table->newTemp(basicType, "");
        code.push_back(new Quadruple(term.value, resultPlace));
    } else if (term.type == "string") {
        value = term.value;
        basicType = BasicType::CHARACTER;
        extended_type = "a";
        resultPlace = s_table->newTemp(basicType, extended_type);
        code.push_back(new Quadruple(term.value, resultPlace));
    } else {
        semanticError("bad term literal: " + term.value);
    }
}

Term::~Term() {
}

void Term::accept(AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

}
