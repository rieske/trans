#include "DeclarationSpecifiers.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/Type.h"

#include <sstream>

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

const std::vector<TypeSpecifier>& DeclarationSpecifiers::getTypeSpecifiers() const {
    return typeSpecifiers;
}

const std::vector<type::Qualifier>& DeclarationSpecifiers::getTypeQualifiers() const {
    return typeQualifiers;
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
        int& longCount, bool& hasFloat, bool& hasDouble, bool& hasVoid) {
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
    type::Type complexType = type::voidType();
    bool hasComplex = false;

    // Tokenize each TypeSpecifier name so multi-word packages from type_name
    // combine (e.g. "unsigned int") still contribute bare keywords in any order.
    for (const auto& ts : typeSpecifiers) {
        const std::string& n = ts.getName();
        if (n.empty()) {
            hasComplex = true;
            complexType = ts.getType();
            continue;
        }
        std::istringstream iss { n };
        std::string tok;
        bool anyKeyword = false;
        while (iss >> tok) {
            if (applyKeywordToken(tok, hasUnsigned, hasSigned, hasChar, hasShort, hasInt, longCount,
                        hasFloat, hasDouble, hasVoid)) {
                anyKeyword = true;
            } else {
                // Non-keyword token (tag / typedef name / "struct" etc.): use stored Type.
                hasComplex = true;
                complexType = ts.getType();
            }
        }
        (void)anyKeyword;
    }

    // Struct/union/enum/typedef without keyword mix: return stored type.
    if (hasComplex && !hasUnsigned && !hasSigned && !hasChar && !hasShort && !hasInt
            && longCount == 0 && !hasFloat && !hasDouble && !hasVoid) {
        if (!typeQualifiers.empty() && complexType.isPrimitive()) {
            return type::primitive(complexType.getPrimitive(), typeQualifiers);
        }
        return complexType;
    }
    // Keyword + complex together (e.g. invalid "unsigned struct S"): prefer keyword path;
    // full constraint diagnostics deferred.
    if (hasVoid) {
        return type::voidType();
    }
    if (hasFloat) {
        return type::floating(typeQualifiers);
    }
    if (hasDouble) {
        return longCount > 0 ? type::longDoubleFloating(typeQualifiers) : type::doubleFloating(typeQualifiers);
    }
    if (hasChar) {
        return hasUnsigned ? type::unsignedCharacter(typeQualifiers) : type::signedCharacter(typeQualifiers);
    }
    if (hasShort) {
        return hasUnsigned ? type::unsignedShort(typeQualifiers) : type::signedShort(typeQualifiers);
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
    return typeSpecifiers.at(0).getType();
}

} // namespace ast
