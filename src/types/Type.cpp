#include "Type.h"

#include <stdexcept>

namespace type {

static const int POINTER_SIZE {8};

Type Type::primitive(const Primitive& primitive, const std::vector<TypeQualifier>& qualifiers) {
    return Type{primitive, qualifiers};
}

Type Type::pointer(const Type& pointsTo, const std::vector<TypeQualifier>& qualifiers) {
    auto p = Type{pointsTo};
    p._indirection = 1;
    return p;
}

Type Type::function(const Type& returnType, const std::vector<Type>& arguments) {
    return Type{returnType, arguments};
}

Type::Type(std::vector<TypeQualifier> qualifiers) {
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

Type::Type(const Primitive& primitive, std::vector<TypeQualifier> qualifiers):
    Type{qualifiers}
{
    _primitive.emplace(primitive);
}

Type::Type(const Type& returnType, const std::vector<Type>& arguments):
    _size{0}
{
    _function.emplace(Function{std::make_unique<Type>(returnType)});
}

int Type::getSize() const {
    if (isPointer()) {
        return POINTER_SIZE;
    }
    if (isPrimitive()) {
        return _primitive.value().getSize();
    }

    return _size;
}

bool Type::isPrimitive() const {
    return _primitive.has_value();
}

Primitive Type::getPrimitive() const {
    return _primitive.value();
}

bool Type::isConst() const {
    return _const;
}

bool Type::isVolatile() const {
    return _volatile;
}

bool Type::isPointer() const {
    return _indirection;
}

bool Type::isFunction() const {
    return _function.has_value();
}

Type Type::getReturnType() const {
    return _function.value().getReturnType();
}

Type Type::dereference() const {
    if (!isPointer()) {
        throw std::domain_error{"can not dereference non-pointer type"};
    }
    Type pointsTo = Type{*this};
    --pointsTo._indirection;
    return pointsTo;
}

} // namespace type

