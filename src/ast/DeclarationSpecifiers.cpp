#include "DeclarationSpecifiers.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/Type.h"

#include <sstream>
#include <stdexcept>

namespace ast {

DeclarationSpecifiers::DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    typeSpecifiers.push_back(typeSpecifier);
}

DeclarationSpecifiers::DeclarationSpecifiers(type::Qualifier typeQualifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    typeQualifiers.push_back(typeQualifier);
}

DeclarationSpecifiers::DeclarationSpecifiers(StorageSpecifier storageSpecifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    storageSpecifiers.push_back(storageSpecifier);
}

DeclarationSpecifiers DeclarationSpecifiers::none() {
    return {};
}

void DeclarationSpecifiers::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void DeclarationSpecifiers::resolveTypeof(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& specifier : typeSpecifiers) {
        specifier.resolveTypeof(visitor);
    }
}

bool DeclarationSpecifiers::resolveTypeofAtParseTime(const ParseEnvironment& environment) {
    bool ok = true;
    for (auto& specifier : typeSpecifiers) {
        if (!specifier.resolveTypeofAtParseTime(environment)) {
            ok = false;
        }
    }
    return ok;
}

bool DeclarationSpecifiers::needsSemanticResolve() const {
    for (const auto& specifier : typeSpecifiers) {
        if (specifier.needsSemanticResolve()) {
            return true;
        }
    }
    return false;
}

const std::vector<TypeSpecifier>& DeclarationSpecifiers::getTypeSpecifiers() const {
    return typeSpecifiers;
}

const std::vector<StorageSpecifier>& DeclarationSpecifiers::getStorageSpecifiers() const {
    return storageSpecifiers;
}

bool DeclarationSpecifiers::hasStorage(Storage storage) const {
    for (const auto& s : storageSpecifiers) {
        if (s.getStorage() == storage) {
            return true;
        }
    }
    return false;
}

namespace {

// True when token is a C type-specifier keyword we fold (not a tag/typedef name).
bool applyKeywordToken(const std::string& tok,
        bool& hasUnsigned, bool& hasSigned, bool& hasChar, bool& hasShort, bool& hasInt,
        int& longCount, bool& hasFloat, bool& hasDouble, bool& hasVoid, bool& hasInt128,
        bool& hasComplexSpec) {
    if (tok == "unsigned") {
        hasUnsigned = true;
        return true;
    }
    if (tok == "signed") {
        hasSigned = true;
        return true;
    }
    if (tok == "char") {
        hasChar = true;
        return true;
    }
    if (tok == "short") {
        hasShort = true;
        return true;
    }
    if (tok == "int") {
        hasInt = true;
        return true;
    }
    if (tok == "long") {
        ++longCount;
        return true;
    }
    if (tok == "float") {
        hasFloat = true;
        return true;
    }
    if (tok == "double") {
        hasDouble = true;
        return true;
    }
    if (tok == "void") {
        hasVoid = true;
        return true;
    }
    if (tok == "__int128") {
        hasInt128 = true;
        return true;
    }
    if (tok == "_Complex") {
        hasComplexSpec = true;
        return true;
    }
    return false;
}

} // namespace

type::Type DeclarationSpecifiers::getResolvedType() const {
    bool hasUnsigned = false;
    bool hasSigned = false;
    bool hasChar = false;
    bool hasShort = false;
    bool hasInt = false;
    int longCount = 0;
    bool hasFloat = false;
    bool hasDouble = false;
    bool hasVoid = false;
    bool hasInt128 = false;
    type::Type compoundType = type::voidType();
    bool hasCompound = false;
    bool hasComplexSpec = false;

    // Tokenize each TypeSpecifier name so multi-word packages from type_name
    // combine (e.g. "unsigned int") still contribute bare keywords in any order.
    for (const auto& ts : typeSpecifiers) {
        const std::string& n = ts.getName();
        if (n.empty()) {
            hasCompound = true;
            if (ts.hasType()) {
                compoundType = ts.getType();
            }
            continue;
        }
        std::istringstream iss { n };
        std::string tok;
        bool anyKeyword = false;
        while (iss >> tok) {
            if (applyKeywordToken(tok, hasUnsigned, hasSigned, hasChar, hasShort, hasInt, longCount,
                        hasFloat, hasDouble, hasVoid, hasInt128, hasComplexSpec)) {
                anyKeyword = true;
            } else {
                // Non-keyword token (tag / typedef name / "struct" etc.): use stored Type.
                hasCompound = true;
                if (ts.hasType()) {
                    compoundType = ts.getType();
                }
            }
        }
        (void)anyKeyword;
    }

    // Struct/union/enum/typedef without keyword mix: return stored type.
    if (hasCompound && !hasUnsigned && !hasSigned && !hasChar && !hasShort && !hasInt
            && longCount == 0 && !hasFloat && !hasDouble && !hasVoid && !hasInt128
            && !hasComplexSpec) {
        if (typeQualifiers.empty()) {
            return compoundType;
        }
        return compoundType.withQualifiers(typeQualifiers);
    }
    // Packaged primitive from spec_qualifier_list combine (e.g. "short int") plus
    // an outer unsigned/signed must still apply (git diff_filepair score).
    if (hasCompound && compoundType.isPrimitive() && !compoundType.getPrimitive().isFloating()) {
        if (!hasUnsigned && !hasSigned) {
            hasUnsigned = !compoundType.getPrimitive().isSigned();
            hasSigned = compoundType.getPrimitive().isSigned();
        }
        using type::PrimitiveKind;
        switch (compoundType.getPrimitive().kind()) {
        case PrimitiveKind::Boolean:
            return typeQualifiers.empty() ? compoundType : compoundType.withQualifiers(typeQualifiers);
        case PrimitiveKind::SignedChar:
        case PrimitiveKind::UnsignedChar:
            hasChar = true;
            break;
        case PrimitiveKind::SignedShort:
        case PrimitiveKind::UnsignedShort:
            hasShort = true;
            break;
        case PrimitiveKind::SignedInteger:
        case PrimitiveKind::UnsignedInteger:
            hasInt = true;
            break;
        case PrimitiveKind::SignedLong:
        case PrimitiveKind::UnsignedLong:
            if (longCount < 1) {
                longCount = 1;
            }
            break;
        case PrimitiveKind::SignedInt128:
        case PrimitiveKind::UnsignedInt128:
            hasInt128 = true;
            break;
        default:
            break;
        }
    }
    // Keyword + compound together (e.g. invalid "unsigned struct S"): prefer keyword path;
    // full constraint diagnostics deferred.
    if (hasVoid) {
        return type::voidType();
    }
    if (hasFloat) {
        return hasComplexSpec ? type::complexFloat(typeQualifiers) : type::floating(typeQualifiers);
    }
    if (hasDouble) {
        if (longCount > 0) {
            return hasComplexSpec ? type::complexLongDouble(typeQualifiers)
                    : type::longDoubleFloating(typeQualifiers);
        }
        return hasComplexSpec ? type::complexDouble(typeQualifiers) : type::doubleFloating(typeQualifiers);
    }
    if (hasComplexSpec) {
        return longCount > 0 ? type::complexLongDouble(typeQualifiers)
                : type::complexDouble(typeQualifiers);
    }
    if (hasChar) {
        return hasUnsigned ? type::unsignedCharacter(typeQualifiers) : type::signedCharacter(typeQualifiers);
    }
    if (hasShort) {
        return hasUnsigned ? type::unsignedShort(typeQualifiers) : type::signedShort(typeQualifiers);
    }
    if (hasInt128) {
        return hasUnsigned ? type::unsignedInt128(typeQualifiers) : type::signedInt128(typeQualifiers);
    }
    if (longCount > 0) {
        return hasUnsigned ? type::unsignedLong(typeQualifiers) : type::signedLong(typeQualifiers);
    }
    if (hasUnsigned) {
        return type::unsignedInteger(typeQualifiers);
    }
    if (hasSigned || hasInt || typeSpecifiers.empty()) {
        return type::signedInteger(typeQualifiers);
    }
    // Unknown non-keyword-only list: fall back to first stored type.
    if (!typeSpecifiers.at(0).hasType()) {
        return type::voidType();
    }
    return typeSpecifiers.at(0).getType();
}

bool DeclarationSpecifiers::isUntaggedCompleteRecord() const {
    if (typeSpecifiers.size() != 1) {
        return false;
    }
    const auto& typeSpecifier = typeSpecifiers.front();
    return typeSpecifier.getName().empty() && typeSpecifier.hasType()
            && typeSpecifier.getType().isRecord() && typeSpecifier.getType().isCompleteRecord();
}

TypeSpecifier DeclarationSpecifiers::toTypeSpecifier() const {
    if (typeSpecifiers.empty()) {
        throw std::invalid_argument { "toTypeSpecifier with no type specifier" };
    }
    if (typeSpecifiers.size() == 1 && typeQualifiers.empty()) {
        return typeSpecifiers.front();
    }
    std::string name = typeSpecifiers.size() == 1 ? typeSpecifiers.front().getName() : std::string {};
    return TypeSpecifier { getResolvedType(), std::move(name) };
}

} // namespace ast
