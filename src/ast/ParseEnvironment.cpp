#include "ParseEnvironment.h"

#include <stdexcept>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "AssignmentExpression.h"
#include "ConditionalExpression.h"
#include "ExpressionList.h"
#include "FunctionCall.h"
#include "IdentifierExpression.h"
#include "MemberAccess.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "TypeCast.h"
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
    if (session_.consumePendingTransparentUnion() && type.isUnion()) {
        type.markTransparentUnion();
    }
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
            return type::enumUnderlyingType(enumValue, enumValue);
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
    // C: type of assignment is the (converted) type of the left operand.
    if (auto* assign = dynamic_cast<const AssignmentExpression*>(&expression)) {
        auto left = typeOf(*assign->getLeftOperand());
        if (!left) {
            return std::nullopt;
        }
        return type::afterLvalueConversion(*left);
    }
    // C: comma yields the type of the right operand.
    if (auto* comma = dynamic_cast<const ExpressionList*>(&expression)) {
        return typeOf(*comma->getRightOperand());
    }
    if (auto* cast = dynamic_cast<const TypeCast*>(&expression)) {
        TypeSpecifier target = cast->getTypeSpecifier();
        if (!target.resolveTypeofAtParseTime(*this) || !target.hasType()) {
            return std::nullopt;
        }
        return target.getType();
    }
    if (auto* call = dynamic_cast<const FunctionCall*>(&expression)) {
        if (const type::Type* builtin = call->builtinTypeArgument()) {
            return *builtin;
        }
        auto callee = typeOf(*call->getOperandExpression());
        if (!callee) {
            return std::nullopt;
        }
        // Lvalue conversion decays bare functions to pointer-to-function.
        const type::Type decayed = type::afterLvalueConversion(*callee);
        if (!decayed.isPointer()) {
            return std::nullopt;
        }
        const type::Type fn = decayed.dereference();
        if (!fn.isFunction()) {
            return std::nullopt;
        }
        return fn.getFunction().getReturnType();
    }
    if (auto* conditional = dynamic_cast<const ConditionalExpression*>(&expression)) {
        auto trueType = typeOf(*conditional->getTrueExpression());
        auto falseType = typeOf(*conditional->getFalseExpression());
        if (!trueType || !falseType) {
            return std::nullopt;
        }
        return type::conditionalResultType(*trueType, *falseType);
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
            (void)session_.consumePendingTransparentUnion();
            return;
        }
        auto baseType = specs.getResolvedType();
        for (const auto& declarator : declarators) {
            type::Type aliased = declarator->getFundamentalType(baseType);
            defineTypedef(declarator->getName(), aliased);
        }
        // Attribute on a typedef with no declarators, or non-union typedefs already
        // consumed in defineTypedef; drop any leftover pending bit.
        if (declarators.empty()) {
            (void)session_.consumePendingTransparentUnion();
        }
        return;
    }
    // transparent_union only applies to union typedefs; discard on ordinary declarations
    // so a prior attribute cannot mark a later union typedef.
    (void)session_.consumePendingTransparentUnion();
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

void ParseEnvironment::addEnumerator(std::string name, std::optional<long> explicitValue) {
    long value = explicitValue ? *explicitValue : (enumBody_ ? enumBody_->next : 0L);
    // Any redefinition of an enumerator name is a constraint violation (C),
    // including same-value and names introduced by other enums / structs.
    long existing = 0;
    if (session_.enums.lookup(name, existing)) {
        throw std::runtime_error { "redefinition of enumerator `" + name + "`" };
    }
    // Register immediately so later enumerators can fold prior names.
    session_.enums.add(name, value);
    if (!enumBody_) {
        enumBody_ = EnumBody { value + 1, value, value };
    } else {
        if (value < enumBody_->min) {
            enumBody_->min = value;
        }
        if (value > enumBody_->max) {
            enumBody_->max = value;
        }
        enumBody_->next = value + 1;
    }
}

bool ParseEnvironment::lookupEnumConstant(const std::string& name, long& value) const {
    return session_.enums.lookup(name, value);
}

type::Type ParseEnvironment::endEnumDefinition(const std::string& tag) {
    // Idempotent: empty enum bodies never call addEnumerator (still OK to end).
    type::Type underlying = type::signedInteger();
    if (enumBody_) {
        underlying = type::enumUnderlyingType(enumBody_->min, enumBody_->max);
        enumBody_.reset();
    }
    if (!tag.empty()) {
        enumTags_.insert_or_assign(tag, underlying);
    }
    return underlying;
}

std::optional<type::Type> ParseEnvironment::lookupEnumTag(const std::string& tag) const {
    for (const ParseEnvironment* env = this; env != nullptr; env = env->tagParent_) {
        auto it = env->enumTags_.find(tag);
        if (it != env->enumTags_.end()) {
            return it->second;
        }
    }
    return std::nullopt;
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
