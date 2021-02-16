#include "TypeCast.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string TypeCast::ID { "<cast_exp>" };

TypeCast::TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression { std::move(castExpression), std::unique_ptr<Operator> { new Operator(typeSpecifier.getName()) } }, typeSpecifier {
                typeSpecifier } {
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

TypeSpecifier TypeCast::getType() const {
    return typeSpecifier;
}

} // namespace ast

