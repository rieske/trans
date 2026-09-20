#include <algorithm>

#include "DeclarationSpecifiers.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

#include <stdexcept>
#include <string_view>

namespace ast {

namespace {

constexpr unsigned kKwUnsigned = 1u << 0;
constexpr unsigned kKwSigned = 1u << 1;
constexpr unsigned kKwChar = 1u << 2;
constexpr unsigned kKwShort = 1u << 3;
constexpr unsigned kKwInt = 1u << 4;
constexpr unsigned kKwFloat = 1u << 5;
constexpr unsigned kKwDouble = 1u << 6;
constexpr unsigned kKwVoid = 1u << 7;
constexpr unsigned kKwInt128 = 1u << 8;
constexpr unsigned kKwComplex = 1u << 9;
constexpr unsigned kKwAnyKeyword = 1u << 10;
constexpr unsigned kKwCompound = 1u << 11;

bool applyKeywordToken(std::string_view tok, unsigned& bits, unsigned& longCount) {
    if (tok == "unsigned") {
        bits |= kKwUnsigned | kKwAnyKeyword;
        return true;
    }
    if (tok == "signed") {
        bits |= kKwSigned | kKwAnyKeyword;
        return true;
    }
    if (tok == "char") {
        bits |= kKwChar | kKwAnyKeyword;
        return true;
    }
    if (tok == "short") {
        bits |= kKwShort | kKwAnyKeyword;
        return true;
    }
    if (tok == "int") {
        bits |= kKwInt | kKwAnyKeyword;
        return true;
    }
    if (tok == "long") {
        ++longCount;
        bits |= kKwAnyKeyword;
        return true;
    }
    if (tok == "float") {
        bits |= kKwFloat | kKwAnyKeyword;
        return true;
    }
    if (tok == "double") {
        bits |= kKwDouble | kKwAnyKeyword;
        return true;
    }
    if (tok == "void") {
        bits |= kKwVoid | kKwAnyKeyword;
        return true;
    }
    if (tok == "__int128") {
        bits |= kKwInt128 | kKwAnyKeyword;
        return true;
    }
    if (tok == "_Complex") {
        bits |= kKwComplex | kKwAnyKeyword;
        return true;
    }
    return false;
}

void ingestSpecifierName(std::string_view name, unsigned& bits, unsigned& longCount) {
    if (name.empty()) {
        bits |= kKwCompound;
        return;
    }
    std::size_t i = 0;
    while (i < name.size()) {
        while (i < name.size() && name[i] == ' ') {
            ++i;
        }
        if (i >= name.size()) {
            break;
        }
        std::size_t j = i;
        while (j < name.size() && name[j] != ' ') {
            ++j;
        }
        if (!applyKeywordToken(name.substr(i, j - i), bits, longCount)) {
            bits |= kKwCompound;
        }
        i = j;
    }
}

} // namespace

DeclarationSpecifiers::DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers rest) :
        DeclarationSpecifiers(std::move(rest)) {
    add(std::move(typeSpecifier));
}

DeclarationSpecifiers::DeclarationSpecifiers(type::Qualifier typeQualifier, DeclarationSpecifiers rest) :
        DeclarationSpecifiers(std::move(rest)) {
    add(typeQualifier);
}

DeclarationSpecifiers::DeclarationSpecifiers(StorageSpecifier storageSpecifier, DeclarationSpecifiers rest) :
        DeclarationSpecifiers(std::move(rest)) {
    add(std::move(storageSpecifier));
}

DeclarationSpecifiers::DeclarationSpecifiers(FunctionSpecifier functionSpecifier, DeclarationSpecifiers rest) :
        DeclarationSpecifiers(std::move(rest)) {
    add(std::move(functionSpecifier));
}

DeclarationSpecifiers DeclarationSpecifiers::none() {
    return {};
}

void DeclarationSpecifiers::add(TypeSpecifier typeSpecifier) {
    ingestSpecifierName(typeSpecifier.getName(), specifierKeywords_, longCount_);
    typeSpecifiers.push_back(std::move(typeSpecifier));
}

void DeclarationSpecifiers::add(type::Qualifier typeQualifier) {
    typeQualifiers.push_back(typeQualifier);
}

void DeclarationSpecifiers::add(StorageSpecifier storageSpecifier) {
    storageSpecifiers.push_back(std::move(storageSpecifier));
}

void DeclarationSpecifiers::add(FunctionSpecifier functionSpecifier) {
    functionSpecifiers.push_back(std::move(functionSpecifier));
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

std::optional<type::Type> DeclarationSpecifiers::typeAtParseTime(const ParseEnvironment& environment) const {
    if (!needsSemanticResolve()) {
        return getResolvedType();
    }
    if (typeSpecifiers.size() != 1) {
        return std::nullopt;
    }
    auto parsed = typeSpecifiers.front().typeAtParseTime(environment);
    if (!parsed) {
        return std::nullopt;
    }
    return parsed->withQualifiers(typeQualifiers);
}

bool DeclarationSpecifiers::needsSemanticResolve() const {
    for (const auto& specifier : typeSpecifiers) {
        if (specifier.needsSemanticResolve()) {
            return true;
        }
    }
    return false;
}

std::vector<TypeSpecifier>& DeclarationSpecifiers::getTypeSpecifiers() {
    return typeSpecifiers;
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

const std::vector<FunctionSpecifier>& DeclarationSpecifiers::getFunctionSpecifiers() const {
    return functionSpecifiers;
}

bool DeclarationSpecifiers::hasFunctionSpec(FunctionSpec spec) const {
    for (const auto& s : functionSpecifiers) {
        if (s.getSpec() == spec) {
            return true;
        }
    }
    return false;
}

type::Type DeclarationSpecifiers::getResolvedType() const {
    const bool hasUnsigned = (specifierKeywords_ & kKwUnsigned) != 0;
    const bool hasSigned = (specifierKeywords_ & kKwSigned) != 0;
    const bool hasChar = (specifierKeywords_ & kKwChar) != 0;
    const bool hasShort = (specifierKeywords_ & kKwShort) != 0;
    const bool hasInt = (specifierKeywords_ & kKwInt) != 0;
    const bool hasFloat = (specifierKeywords_ & kKwFloat) != 0;
    const bool hasDouble = (specifierKeywords_ & kKwDouble) != 0;
    const bool hasVoid = (specifierKeywords_ & kKwVoid) != 0;
    const bool hasInt128 = (specifierKeywords_ & kKwInt128) != 0;
    const bool hasComplexSpec = (specifierKeywords_ & kKwComplex) != 0;
    const bool hasCompound = (specifierKeywords_ & kKwCompound) != 0;
    const bool anyKeyword = (specifierKeywords_ & kKwAnyKeyword) != 0;

    if (hasCompound && !anyKeyword) {
        type::Type compoundType = type::voidType();
        for (const auto& ts : typeSpecifiers) {
            if (ts.hasType()) {
                compoundType = ts.getType();
            }
        }
        return compoundType.withQualifiers(typeQualifiers);
    }
    if (hasVoid) {
        return type::voidType();
    }
    if (hasFloat) {
        return hasComplexSpec ? type::complexFloat(typeQualifiers) : type::floating(typeQualifiers);
    }
    if (hasDouble) {
        if (longCount_ > 0) {
            return hasComplexSpec ? type::complexLongDouble(typeQualifiers)
                    : type::longDoubleFloating(typeQualifiers);
        }
        return hasComplexSpec ? type::complexDouble(typeQualifiers) : type::doubleFloating(typeQualifiers);
    }
    if (hasComplexSpec) {
        return longCount_ > 0 ? type::complexLongDouble(typeQualifiers)
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
    if (longCount_ > 0) {
        return hasUnsigned ? type::unsignedLong(typeQualifiers) : type::signedLong(typeQualifiers);
    }
    if (hasUnsigned) {
        return type::unsignedInteger(typeQualifiers);
    }
    if (hasSigned || hasInt || typeSpecifiers.empty()) {
        return type::signedInteger(typeQualifiers);
    }
    if (!typeSpecifiers.at(0).hasType()) {
        return type::voidType();
    }
    return typeSpecifiers.at(0).getType();
}

bool DeclarationSpecifiers::isUntaggedRecordBody() const {
    if (typeSpecifiers.size() != 1) {
        return false;
    }
    const auto& typeSpecifier = typeSpecifiers.front();
    return typeSpecifier.getName().empty() && typeSpecifier.hasType()
            && typeSpecifier.getType().isRecord()
            && (typeSpecifier.getType().isCompleteRecord()
                    || type::isTentativeRecord(typeSpecifier.getType()));
}

TypeSpecifier DeclarationSpecifiers::toTypeSpecifier() const {
    if (typeSpecifiers.empty()) {
        throw std::invalid_argument { "toTypeSpecifier with no type specifier" };
    }
    const TypeSpecifier& front = typeSpecifiers.front();
    const bool single = typeSpecifiers.size() == 1;
    std::string name = single ? front.getName() : std::string {};
    TypeSpecifier merged { getResolvedType(), std::move(name),
            single ? front.getContext() : translation_unit::Context { "", 0 },
            single && front.definesRecord() };
    const auto defining = std::find_if(typeSpecifiers.begin(), typeSpecifiers.end(),
            [](const TypeSpecifier& s) { return !s.enumerators().empty(); });
    if (defining != typeSpecifiers.end()) {
        merged.setEnumerators(defining->enumerators());
    }
    return merged;
}

} // namespace ast
