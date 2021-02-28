#include "Type.h"

#include <stdexcept>

namespace type {

static const int POINTER_SIZE {8};

Type Type::signedCharacter(std::vector<TypeQualifier> qualifiers) {
    return Type{1, true, false, qualifiers};
}

Type Type::unsignedCharacter(std::vector<TypeQualifier> qualifiers) {
    return Type{1, false, false, qualifiers};
}

Type Type::signedInteger(std::vector<TypeQualifier> qualifiers) {
    return Type{4, true, false, qualifiers};
}

Type Type::unsignedInteger(std::vector<TypeQualifier> qualifiers) {
    return Type{4, false, false, qualifiers};
}

Type Type::signedLong(std::vector<TypeQualifier> qualifiers) {
    return Type{8, true, false, qualifiers};
}

Type Type::unsignedLong(std::vector<TypeQualifier> qualifiers) {
    return Type{8, false, false, qualifiers};
}

Type Type::floating(std::vector<TypeQualifier> qualifiers) {
    return Type{4, true, true, qualifiers};
}

Type Type::doubleFloating(std::vector<TypeQualifier> qualifiers) {
    return Type{8, true, true, qualifiers};
}

Type Type::longDoubleFloating(std::vector<TypeQualifier> qualifiers) {
    return Type{16, true, true, qualifiers};
}

Type Type::pointer(const Type& pointsTo, std::vector<TypeQualifier> qualifiers) {
    auto p = Type{pointsTo};
    p._indirection = 1;
    return p;
}

Type::Type(int _size, bool _signed, bool _float, std::vector<TypeQualifier> qualifiers):
    _size{_size},
    _signed{_signed},
    _float{_float}
{
    for (const auto& qualifier: qualifiers) {
        switch(qualifier) {
            case TypeQualifier::CONST:
                this->_const = true;
                break;
            case TypeQualifier::VOLATILE:
                this->_volatile = true;
                break;
            default:
                throw std::invalid_argument{"Unsupported type qualifier"};
        }
    }
}

int Type::getSize() const {
    if (_indirection) {
        return POINTER_SIZE;
    }
    return _size;
}

bool Type::isSigned() const {
    return _signed;
}

bool Type::isFloating() const {
    return _float;
}

bool Type::isConst() const {
    return _const;
}

bool Type::isVolatile() const {
    return _volatile;
}

Type Type::dereference() const {
    if (!_indirection) {
        throw std::domain_error{"can not dereference non-pointer type"};
    }
    Type pointsTo = Type{*this};
    --pointsTo._indirection;
    return pointsTo;
}

} // namespace type

