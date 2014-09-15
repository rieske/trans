#include "Term.h"

#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace ast {

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
        setTypeInfo( { BasicType::CHARACTER, 1 });
    } else {
        throw std::runtime_error("bad term literal: " + term.value);
    }
}

Term::~Term() {
}

void Term::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

const TranslationUnitContext& Term::getContext() const {
    return term.context;
}

std::string Term::getType() const {
    return term.type;
}

std::string Term::getValue() const {
    return term.value;
}

}
