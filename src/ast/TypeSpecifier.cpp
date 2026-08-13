#include "TypeSpecifier.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Declarator.h"
#include "Expression.h"
#include "ParseEnvironment.h"

#include <stdexcept>

namespace ast {

TypeSpecifier::TypeSpecifier(type::Type type, std::string name) :
        name { std::move(name) },
        type { std::move(type) }
{
}

TypeSpecifier::TypeSpecifier(std::shared_ptr<Expression> typeofOperand) :
        typeofOperand_ { std::move(typeofOperand) }
{
}

TypeSpecifier::~TypeSpecifier() = default;
TypeSpecifier::TypeSpecifier(const TypeSpecifier&) = default;
TypeSpecifier& TypeSpecifier::operator=(const TypeSpecifier&) = default;
TypeSpecifier::TypeSpecifier(TypeSpecifier&&) noexcept = default;
TypeSpecifier& TypeSpecifier::operator=(TypeSpecifier&&) noexcept = default;

const std::string& TypeSpecifier::getName() const {
    return name;
}

bool TypeSpecifier::hasType() const {
    return type.has_value();
}

type::Type TypeSpecifier::getType() const {
    if (!type) {
        throw std::runtime_error { "type specifier type is not set" };
    }
    return *type;
}

void TypeSpecifier::dropSpelling() {
    name.clear();
}

void TypeSpecifier::deferAbstractDeclarator(std::unique_ptr<Declarator> declarator) {
    if (!declarator) {
        return;
    }
    if (typeofOperand_ || !type) {
        deferredDeclarator_ = std::shared_ptr<Declarator> { std::move(declarator) };
        return;
    }
    type = declarator->getFundamentalType(*type);
    name.clear();
}

void TypeSpecifier::resolveTypeof(AbstractSyntaxTreeVisitor& visitor) {
    bool operandFailed = false;
    if (typeofOperand_) {
        typeofOperand_->accept(visitor);
        if (typeofOperand_->hasExpressionType()) {
            type = typeofOperand_->expressionType();
        } else {
            operandFailed = true;
        }
        typeofOperand_.reset();
    }
    if (deferredDeclarator_) {
        if (!operandFailed && type) {
            type = deferredDeclarator_->getFundamentalType(*type);
        }
        deferredDeclarator_.reset();
    }
}

bool TypeSpecifier::needsSemanticResolve() const {
    return typeofOperand_ != nullptr || deferredDeclarator_ != nullptr || !type;
}

bool TypeSpecifier::resolveTypeofAtParseTime(const ParseEnvironment& environment) {
    if (typeofOperand_) {
        auto parsed = environment.typeOf(*typeofOperand_);
        if (!parsed) {
            return false;
        }
        type = *parsed;
        typeofOperand_.reset();
    }
    if (deferredDeclarator_) {
        if (!type) {
            return false;
        }
        type = deferredDeclarator_->getFundamentalType(*type);
        deferredDeclarator_.reset();
    }
    return static_cast<bool>(type);
}

} // namespace ast
