#include "ParseEnvironment.h"

#include <stdexcept>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "AssignmentExpression.h"
#include "CompoundLiteralExpression.h"
#include "IdentifierExpression.h"
#include "MemberAccess.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "UnaryExpression.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

namespace ast {

ParseEnvironment::ParseEnvironment(scanner::LexicalSession& session) :
        session_ { session } {
}

ParseEnvironment::ParseEnvironment(scanner::LexicalSession& session, ParseEnvironment& parent) :
        session_ { session },
        tagParent_ { &parent } {
}

type::Type ParseEnvironment::ensureStructTag(const std::string& tag) {
    for (ParseEnvironment* env = this; env != nullptr; env = env->tagParent_) {
        auto it = env->structTags_.find(tag);
        if (it != env->structTags_.end()) {
            return it->second;
        }
    }
    type::Type incomplete = type::incompleteStructure();
    structTags_.emplace(tag, incomplete);
    return incomplete;
}

void ParseEnvironment::defineTypedef(const std::string& name, type::Type type) {
    session_.typedefs.add(name, type);
}

std::optional<type::Type> ParseEnvironment::lookupTypedef(const std::string& name) const {
    return session_.typedefs.tryLookup(name);
}

void ParseEnvironment::defineObject(const std::string& name, type::Type type) {
    session_.objects.add(name, type);
}

void ParseEnvironment::maybeDefineParameter(const FormalArgument& argument) {
    if (argument.getName().empty() || argument.needsSemanticResolve()) {
        return;
    }
    try {
        session_.objects.addPending(argument.getName(), argument.getType());
    } catch (const std::invalid_argument&) {
    }
}

std::optional<type::Type> ParseEnvironment::lookupObject(const std::string& name) const {
    return session_.objects.lookup(name);
}

std::optional<type::Type> ParseEnvironment::typeOf(const Expression& expression) const {
    if (auto* id = dynamic_cast<const IdentifierExpression*>(&expression)) {
        if (auto objectType = lookupObject(id->getIdentifier())) {
            return objectType;
        }
        long enumValue = 0;
        if (lookupEnumConstant(id->getIdentifier(), enumValue)) {
            return type::signedInteger();
        }
        return std::nullopt;
    }
    if (expression.hasExpressionType()) {
        return expression.expressionType();
    }
    if (auto* prefix = dynamic_cast<const PrefixExpression*>(&expression)) {
        return typeOf(*prefix->getOperandExpression());
    }
    if (auto* postfix = dynamic_cast<const PostfixExpression*>(&expression)) {
        return typeOf(*postfix->getOperandExpression());
    }
    if (auto* subscript = dynamic_cast<const ArrayAccess*>(&expression)) {
        auto base = typeOf(*subscript->getLeftOperand());
        if (!base) {
            return std::nullopt;
        }
        return type::afterLvalueConversion(*base).indexElement();
    }
    if (auto* member = dynamic_cast<const MemberAccess*>(&expression)) {
        auto base = typeOf(*member->getBase());
        if (!base) {
            return std::nullopt;
        }
        return type::memberAccessResult(*base, member->isArrow(), member->getMemberName());
    }
    if (auto* arith = dynamic_cast<const ArithmeticExpression*>(&expression)) {
        auto left = typeOf(*arith->getLeftOperand());
        auto right = typeOf(*arith->getRightOperand());
        if (!left || !right) {
            return std::nullopt;
        }
        return type::arithmeticExpressionResult(*left, *right,
                arith->getOperator()->getLexeme().front());
    }
    if (auto* unary = dynamic_cast<const UnaryExpression*>(&expression)) {
        auto inner = typeOf(*unary->getOperandExpression());
        if (!inner) {
            return std::nullopt;
        }
        const std::string op = unary->getOperator()->getLexeme();
        if (op == "*") {
            return type::afterLvalueConversion(*inner).indexElement();
        }
        if (op == "&") {
            return type::pointer(*inner);
        }
        return std::nullopt;
    }
    if (auto* compound = dynamic_cast<const CompoundLiteralExpression*>(&expression)) {
        return compound->getTypeName().tryResolve(*this);
    }
    if (auto* assign = dynamic_cast<const AssignmentExpression*>(&expression)) {
        return typeOf(*assign->getLeftOperand());
    }
    return std::nullopt;
}

void ParseEnvironment::registerInitializedDeclaration(
        const DeclarationSpecifiers& specs,
        const std::vector<std::unique_ptr<InitializedDeclarator>>& declarators) {
    if (specs.isTypedef()) {
        // Incomplete reduction (typedef with no type-specs): no alias to register.
        // Soft-return rather than throw; pin via ParseEnvironment unit test.
        if (specs.getTypeSpecifiers().empty()) {
            return;
        }
        auto baseType = specs.getResolvedType();
        for (const auto& declarator : declarators) {
            type::Type aliased = declarator->getFundamentalType(baseType);
            defineTypedef(declarator->getName(), aliased);
        }
        return;
    }
    for (const auto& declarator : declarators) {
        const std::string& name = declarator->getName();
        if (lookupTypedef(name)) {
            session_.typedefs.addIdentifierShadow(name);
        }
        if (!specs.needsSemanticResolve()) {
            defineObject(name, declarator->getFundamentalType(specs.getResolvedType()));
        }
    }
}

void ParseEnvironment::beginEnumDefinition() {
    enumNextValueStack_.push_back(0L);
}

void ParseEnvironment::addEnumerator(std::string name, std::optional<long> explicitValue) {
    // First enumerator opens the auto-increment window (no separate CSNB begin).
    if (enumNextValueStack_.empty()) {
        beginEnumDefinition();
    }
    long value = explicitValue ? *explicitValue : enumNextValueStack_.back();
    // Any redefinition of an enumerator name is a constraint violation (C),
    // including same-value and names introduced by other enums / structs.
    long existing = 0;
    if (session_.enums.lookup(name, existing)) {
        throw std::runtime_error { "redefinition of enumerator `" + name + "`" };
    }
    // Register immediately so later enumerators can fold prior names.
    session_.enums.add(name, value);
    enumNextValueStack_.back() = value + 1;
}

bool ParseEnvironment::lookupEnumConstant(const std::string& name, long& value) const {
    return session_.enums.lookup(name, value);
}

void ParseEnvironment::endEnumDefinition() {
    // Idempotent: empty enum bodies never call addEnumerator (still OK to end).
    if (!enumNextValueStack_.empty()) {
        enumNextValueStack_.pop_back();
    }
}

std::map<std::string, long> ParseEnvironment::enumConstantsSnapshot() const {
    return session_.enums.entries();
}

void ParseEnvironment::maybeRegisterParameterShadow(const std::string& name) {
    if (name.empty() || !lookupTypedef(name)) {
        return;
    }
    session_.typedefs.addPendingParameterShadow(name);
}

} // namespace ast
