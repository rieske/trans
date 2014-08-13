#include "Term.h"

#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace semantic_analyzer {

const std::string Term::ID { "<term>" };

Term::Term(TerminalSymbol term) :
        term { term } {

    if (term.type == "id") {
        value = "lval";
    } else if (term.type == "int_const") {
        value = "rval";
        basicType = BasicType::INTEGER;
    } else if (term.type == "float_const") {
        value = "rval";
        basicType = BasicType::FLOAT;
    } else if (term.type == "literal") {
        value = "rval";
        basicType = BasicType::CHARACTER;
    } else if (term.type == "string") {
        value = term.value;
        basicType = BasicType::CHARACTER;
        extended_type = "a";
    } else {
        throw std::runtime_error("bad term literal: " + term.value);
    }
}

Term::~Term() {
}

void Term::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
