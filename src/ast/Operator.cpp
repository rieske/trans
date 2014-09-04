#include "Operator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

Operator::Operator(std::string lexeme) :
        lexeme { lexeme } {
}

Operator::~Operator() {
}

void Operator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::string Operator::getLexeme() const {
    return lexeme;
}

} /* namespace ast */
