#include "TypeSpecifier.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Declarator.h"
#include "Expression.h"
#include "ParseEnvironment.h"
#include "VlaExpressionTable.h"

#include <limits>
#include <stdexcept>
#include <vector>

namespace ast {

TypeSpecifier::TypeSpecifier(type::Type type, std::string name, translation_unit::Context context,
        bool definesRecord) :
        name { std::move(name) },
        context_ { std::move(context) },
        type { std::move(type) },
        definesRecord_ { definesRecord }
{
}

TypeSpecifier::TypeSpecifier(std::unique_ptr<Expression> typeofOperand) :
        typeofOperand_ { std::move(typeofOperand) }
{
}

TypeSpecifier::~TypeSpecifier() = default;
TypeSpecifier::TypeSpecifier(TypeSpecifier&&) noexcept = default;
TypeSpecifier& TypeSpecifier::operator=(TypeSpecifier&&) noexcept = default;

const std::string& TypeSpecifier::getName() const {
    return name;
}

const translation_unit::Context& TypeSpecifier::getContext() const {
    return context_;
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

void TypeSpecifier::applyDeclarator() {
    if (!deferredDeclarator_ || !type) {
        return;
    }
    type = deferredDeclarator_->getFundamentalType(*type);
    deferredDeclarator_.reset();
}

void TypeSpecifier::deferAbstractDeclarator(std::unique_ptr<Declarator> declarator) {
    deferredDeclarator_ = std::move(declarator);
    if (!typeofOperand_ && type) {
        name.clear();
        applyDeclarator();
    }
}

void TypeSpecifier::resolveTypeof(AbstractSyntaxTreeVisitor& visitor) {
    if (typeofOperand_) {
        typeofOperand_->accept(visitor);
        if (typeofOperand_->hasExpressionType()) {
            type = typeofOperand_->expressionType();
        }
        typeofOperand_.reset();
    }
    applyDeclarator();
}

bool TypeSpecifier::needsSemanticResolve() const {
    return typeofOperand_ != nullptr || !type;
}

bool TypeSpecifier::resolveTypeofAtParseTime(const ParseEnvironment& environment) {
    auto parsed = typeAtParseTime(environment);
    if (!parsed) {
        return false;
    }
    type = std::move(*parsed);
    typeofOperand_.reset();
    deferredDeclarator_.reset();
    return true;
}

std::optional<type::Type> TypeSpecifier::typeAtParseTime(const ParseEnvironment& environment) const {
    if (type) {
        return *type;
    }
    if (!typeofOperand_) {
        return std::nullopt;
    }
    auto parsed = environment.typeOf(*typeofOperand_);
    if (!parsed) {
        return std::nullopt;
    }
    if (deferredDeclarator_) {
        return deferredDeclarator_->getFundamentalType(*parsed);
    }
    return *parsed;
}

type::Type foldConstantArrayBounds(const type::Type& t, const VlaExpressionTable& exprs) {
    if (!t.isArray()) {
        return t;
    }
    const type::Type elem = foldConstantArrayBounds(t.getElementType(), exprs);
    const type::Type result = [&] {
        if (t.isVariableArray()) {
            auto bound = exprs.lookup(t.vlaBound().get());
            long n = 0;
            if (bound && bound->foldToHostLong(n) && n >= 0
                    && n <= static_cast<long>(std::numeric_limits<int>::max())) {
                return type::array(elem, static_cast<int>(n));
            }
            return type::variableArray(elem, t.vlaBound());
        }
        if (t.isIncompleteArray()) {
            return type::incompleteArray(elem);
        }
        return type::array(elem, t.getArraySize());
    }();
    std::vector<type::Qualifier> quals;
    if (t.isConst()) {
        quals.push_back(type::Qualifier::CONST);
    }
    if (t.isVolatile()) {
        quals.push_back(type::Qualifier::VOLATILE);
    }
    return result.withQualifiers(quals);
}

void TypeSpecifier::setEnumerators(std::vector<Enumerator> enumerators) {
    enumerators_ = std::move(enumerators);
}

const std::vector<Enumerator>& TypeSpecifier::enumerators() const {
    return enumerators_;
}

bool TypeSpecifier::definesRecord() const {
    return definesRecord_;
}

void TypeSpecifier::refoldConstantArrayBounds(const VlaExpressionTable& exprs) {
    if (type) {
        type = foldConstantArrayBounds(*type, exprs);
    }
}

} // namespace ast
