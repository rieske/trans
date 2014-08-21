#include "Term.h"

#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace semantic_analyzer {

const std::string Term::ID { "<term>" };

Term::Term(TerminalSymbol term) :
        term { term } {

    if (term.type == "id") {
        lval = true;
    } else if (term.type == "int_const") {
        setTypeInfo( { BasicType::INTEGER });
    } else if (term.type == "float_const") {
        setTypeInfo( { BasicType::FLOAT });
    } else if (term.type == "literal") {
        setTypeInfo( { BasicType::CHARACTER });
    } else if (term.type == "string") {
        // FIXME:
        throw std::runtime_error { "strings not implemented yet" };
        //value = term.value;
        setTypeInfo( { BasicType::CHARACTER, "a" });
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
