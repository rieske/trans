#include "Term.h"

#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "types/BaseType.h"

namespace ast {

const std::string Term::ID { "<term>" };

Term::Term(TerminalSymbol term) :
        term { term } {

    if (term.type == "id") {
        lval = true;
    } else if (term.type == "int_const") {
        setType( { BaseType::newInteger() });
    } else if (term.type == "float_const") {
        setType( { BaseType::newFloat() });
    } else if (term.type == "literal") {
        setType( { BaseType::newCharacter() });
    } else if (term.type == "string") {
        // FIXME:
        throw std::runtime_error { "strings not implemented yet" };
        setType( { BaseType::newCharacter(), 1 });
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

std::string Term::getTypeSymbol() const {
    return term.type;
}

std::string Term::getValue() const {
    return term.value;
}

}
