#include "ParseEnvironment.h"

#include "Block.h"
#include "Declaration.h"
#include "Expression.h"

namespace ast {

ParseEnvironment::ParseEnvironment(scanner::LexicalSession& session) :
        session_ { session },
        vlas_ { std::make_shared<VlaExpressionTable>() } {
}

ParseEnvironment::ParseEnvironment(scanner::LexicalSession& session, const ParseEnvironment& parent) :
        session_ { session },
        tagParent_ { &parent },
        gnuExtensions_ { parent.gnuExtensions_ },
        vlas_ { parent.vlas_ } {
}

ParseEnvironment ParseEnvironment::nestedIn(const ParseEnvironment& enclosing) {
    return ParseEnvironment { enclosing.session_, enclosing };
}

type::Type ParseEnvironment::ensureStructTag(const std::string& tag) {
    for (const ParseEnvironment* env = this; env != nullptr; env = env->tagParent_) {
        auto it = env->structTags_.find(tag);
        if (it != env->structTags_.end()) {
            return it->second;
        }
    }
    type::Type incomplete = type::incompleteRecord();
    structTags_.emplace(tag, incomplete);
    return incomplete;
}

void ParseEnvironment::defineTypedef(const std::string& name, type::Type type) {
    if (session_.transparentUnion.consume() && type.isUnion()) {
        type.markTransparentUnion();
    }
    session_.names.addTypedef(name, type);
}

std::optional<type::Type> ParseEnvironment::lookupTypedef(const std::string& name) const {
    return session_.names.lookupTypedef(name);
}

void ParseEnvironment::defineObject(const std::string& name, type::Type type) {
    session_.types.add(name, type);
}

void ParseEnvironment::maybeDefineParameter(const FormalArgument& argument) {
    if (argument.getName().empty() || argument.needsSemanticResolve()) {
        return;
    }
    try {
        session_.types.addPending(argument.getName(), argument.getType());
    } catch (const std::invalid_argument&) {
    }
}

std::optional<type::Type> ParseEnvironment::lookupObject(const std::string& name) const {
    return session_.types.lookup(name);
}

std::optional<type::Type> ParseEnvironment::lookupValueType(const std::string& name) const {
    for (const ParseEnvironment* env = this; env != nullptr; env = env->tagParent_) {
        auto found = env->transients_.find(name);
        if (found != env->transients_.end()) {
            return found->second;
        }
    }
    auto binding = innermostOrdinary(name);
    if (binding.objectType) {
        return binding.objectType;
    }
    if (binding.enumerator) {
        return binding.enumerator->type;
    }
    return std::nullopt;
}

void ParseEnvironment::defineTransient(const std::string& name, type::Type type) {
    transients_.insert_or_assign(name, std::move(type));
}

void ParseEnvironment::bindBlockDeclarations(const Block& block) {
    for (const auto& item : block.getItems()) {
        const auto* declaration = item.asDeclaration();
        if (!declaration) {
            continue;
        }
        DeclarationSpecifiers specs = declaration->getDeclarationSpecifiers();
        if (!specs.resolveTypeofAtParseTime(*this) || specs.getTypeSpecifiers().empty()) {
            continue;
        }
        const type::Type base = specs.getResolvedType();
        for (const auto& declarator : declaration->getDeclarators()) {
            if (declarator->getName().empty()) {
                continue;
            }
            defineTransient(declarator->getName(), declarator->getFundamentalType(base));
        }
    }
}

std::optional<type::Type> ParseEnvironment::typeOf(const Expression& expression) const {
    return expression.typeAtParseTime(*this);
}

void ParseEnvironment::registerInitializedDeclaration(
        const DeclarationSpecifiers& specs,
        const std::vector<std::unique_ptr<InitializedDeclarator>>& declarators) {
    if (specs.isTypedef()) {
        // Incomplete reduction (typedef with no type-specs): no alias to register.
        // Soft-return rather than throw; pin via ParseEnvironment unit test.
        if (specs.getTypeSpecifiers().empty()) {
            session_.transparentUnion.discard();
            (void)session_.recordPacked.takeLatePacked();
            return;
        }
        auto baseType = specs.getResolvedType();
        if (session_.recordPacked.takeLatePacked()) {
            baseType.applyPacked();
        }
        for (const auto& declarator : declarators) {
            type::Type aliased = declarator->getFundamentalType(baseType);
            defineTypedef(declarator->getName(), aliased);
        }
        if (declarators.empty()) {
            session_.transparentUnion.discard();
        }
        return;
    }
    const bool latePacked = session_.recordPacked.takeLatePacked();
    if (latePacked && !specs.getTypeSpecifiers().empty()) {
        type::Type baseType = specs.getResolvedType();
        baseType.applyPacked();
    }
    session_.transparentUnion.discard();
    for (const auto& declarator : declarators) {
        const std::string& name = declarator->getName();
        if (lookupTypedef(name)) {
            session_.names.addIdentifierShadow(name);
        }
        tryDefineObject(specs, declarator->getDeclarator());
    }
}

void ParseEnvironment::tryDefineObject(const DeclarationSpecifiers& specs, Declarator& declarator) {
    if (specs.needsSemanticResolve() || declarator.getName().empty()) {
        return;
    }
    try {
        defineObject(declarator.getName(), declarator.getFundamentalType(specs.getResolvedType()));
    } catch (const std::invalid_argument&) {
    }
}

bool ParseEnvironment::enumeratorInCurrentScope(const std::string& name) const {
    return session_.enums.containsInCurrentScope(name);
}

bool ParseEnvironment::addEnumerator(std::string name) {
    if (auto next = session_.enums.nextInCurrentBody()) {
        return addEnumerator(std::move(name), *next);
    }
    return addEnumerator(std::move(name), type::fromLiteralBits(0, type::signedInteger()));
}

bool ParseEnvironment::addEnumerator(std::string name, type::IntegerConstant value) {
    if (session_.enums.containsInCurrentScope(name)
            || session_.types.containsInCurrentScope(name)) {
        return false;
    }
    session_.enums.add(name, value);
    session_.enums.recordInCurrentBody(std::move(name), value);
    return true;
}

bool ParseEnvironment::lookupEnumConstant(const std::string& name,
        type::IntegerConstant& value) const {
    return session_.enums.lookup(name, value);
}

ParseEnvironment::OrdinaryBinding ParseEnvironment::innermostOrdinary(const std::string& name) const {
    OrdinaryBinding binding;
    const auto objectDepth = session_.types.bindingDepth(name);
    const auto enumeratorDepth = session_.enums.bindingDepth(name);
    if (objectDepth && (!enumeratorDepth || *objectDepth >= *enumeratorDepth)) {
        binding.objectType = lookupObject(name);
        return binding;
    }
    type::IntegerConstant ice;
    if (enumeratorDepth && session_.enums.lookup(name, ice)) {
        binding.enumerator = ice;
    }
    return binding;
}

bool ParseEnvironment::lookupInnermostEnumerator(const std::string& name,
        type::IntegerConstant& value) const {
    auto binding = innermostOrdinary(name);
    if (!binding.enumerator) {
        return false;
    }
    value = *binding.enumerator;
    return true;
}

ParseEnvironment::ClosedEnum ParseEnvironment::endEnumDefinition(const std::string& tag) {
    scanner::ClosedEnumBody body = session_.enums.closeBody();
    ClosedEnum closed;
    if (body.hasRange) {
        closed.underlying = type::enumUnderlyingType(body.min, body.max);
    }
    closed.enumerators.reserve(body.enumerators.size());
    for (auto& enumerator : body.enumerators) {
        closed.enumerators.push_back(
                Enumerator { std::move(enumerator.first), std::move(enumerator.second) });
    }
    if (!tag.empty()) {
        enumTags_.insert_or_assign(tag, closed.underlying);
    }
    return closed;
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

// Token-class hide for a parameter that reuses a typedef spelling. Not an object.
void ParseEnvironment::maybeRegisterParameterShadow(const std::string& name) {
    if (name.empty() || !lookupTypedef(name)) {
        return;
    }
    session_.names.addPendingParameterShadow(name);
}

} // namespace ast
