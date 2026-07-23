#include "DeclarationSpecifiers.h"

#include "AbstractSyntaxTreeVisitor.h"

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

    for (const auto& ts : typeSpecifiers) {
        const std::string& n = ts.getName();
        if (n == "unsigned") {
            hasUnsigned = true;
        } else if (n == "signed") {
            hasSigned = true;
        } else if (n == "char") {
            hasChar = true;
        } else if (n == "short") {
            hasShort = true;
        } else if (n == "int") {
            hasInt = true;
        } else if (n == "long") {
            ++longCount;
        } else if (n == "float") {
            hasFloat = true;
        } else if (n == "double") {
            hasDouble = true;
        } else if (n == "void") {
            hasVoid = true;
        } else {
            // typedef name, struct/union/enum type_spec - use stored type.
            hasComplex = true;
            complexType = ts.getType();
        }
    }

    if (hasComplex) {
        // Multi-word spec_qualifier_list combine packages e.g. `short int` into one
        // TypeSpecifier named "short int". Outer `unsigned` must still apply
        // (git diff_filepair: unsigned short int score) rather than treating the
        // intermediate as an opaque typedef and dropping signedness.
        if (complexType.isPrimitive() && !complexType.getPrimitive().isFloating()
                && (hasUnsigned || hasSigned)) {
            const int sz = complexType.getPrimitive().getSize();
            if (sz == 1) {
                hasChar = true;
            } else if (sz == 2) {
                hasShort = true;
            } else if (sz == 4) {
                hasInt = true;
            } else if (sz == 8) {
                if (longCount < 1) {
                    longCount = 1;
                }
            } else {
                if (!typeQualifiers.empty()) {
                    return type::primitive(complexType.getPrimitive(), typeQualifiers);
                }
                return complexType;
            }
            // Fall through to signedness-aware resolution below.
        } else {
            if (!typeQualifiers.empty() && complexType.isPrimitive()) {
                return type::primitive(complexType.getPrimitive(), typeQualifiers);
            }
            return complexType;
        }
    }
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
        // C ABI short is 2 bytes (ctype tables, packed structs).
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
    // Fallback: first type specifier's type (should be unreachable for valid C).
    return typeSpecifiers.at(0).getType();
}

} // namespace ast
