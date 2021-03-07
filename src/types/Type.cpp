#include "Type.h"

#include <stdexcept>
#include <sstream>

namespace type {

static const int POINTER_SIZE {8};

Type voidType() {
    return Type{{}};
}

Type primitive(const Primitive& primitive, const std::vector<TypeQualifier>& qualifiers) {
    return Type{primitive, qualifiers};
}

Type pointer(const Type& pointsTo, const std::vector<TypeQualifier>& qualifiers) {
    auto p = Type{pointsTo};
    p._indirection = pointsTo._indirection+1;
    return p;
}

Type function(const Type& returnType, const std::vector<Type>& arguments) {
    return Type{returnType, arguments};
}

Type signedCharacter(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::signedCharacter(), qualifiers);
}
Type unsignedCharacter(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::unsignedCharacter(), qualifiers);
}

Type signedInteger(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::signedInteger(), qualifiers);
}

Type unsignedInteger(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::unsignedInteger(), qualifiers);
}

Type signedLong(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::signedLong(), qualifiers);
}

Type unsignedLong(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::unsignedLong(), qualifiers);
}

Type floating(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::floating(), qualifiers);
}

Type doubleFloating(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::doubleFloating(), qualifiers);
}

Type longDoubleFloating(const std::vector<TypeQualifier>& qualifiers) {
    return primitive(Primitive::longDoubleFloating(), qualifiers);
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
    _primitive = primitive;
}

Type::Type(const Type& returnType, const std::vector<Type>& arguments) {
    std::vector<std::unique_ptr<Type>> args;
    for (const auto& arg : arguments) {
        args.push_back(std::make_unique<Type>(arg));
    }
    _function = Function{std::make_unique<Type>(returnType), std::move(args)};
}

int Type::getSize() const {
    if (isPointer()) {
        return POINTER_SIZE;
    }
    if (isPrimitive()) {
        return _primitive->getSize();
    }

    return _size;
}

bool Type::canAssignFrom(const Type& other) const {
    // TODO:
    return true;
}

bool Type::isVoid() const {
    return !isPrimitive() && !isFunction() && !isPointer() && !isStructure();
}

bool Type::isPrimitive() const {
    return _primitive.has_value();
}

Primitive Type::getPrimitive() const {
    return *_primitive;
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

Function Type::getFunction() const {
    return *_function;
}

bool Type::isStructure() const {
    return false;
}

Type Type::dereference() const {
    if (!isPointer()) {
        throw std::domain_error{"can not dereference non-pointer type"};
    }
    Type pointsTo = Type{*this};
    --pointsTo._indirection;
    return pointsTo;
}

std::string Type::to_string() const {
    if (isVoid()) {
        return "void";
    }
    if (isPointer()) {
        return dereference().to_string() + "*";
    }
    if (isPrimitive()) {
        std::stringstream str;
        if (isConst()) {
            str << "const ";
        }
        if (isVolatile()) {
            str << "volatile ";
        }
        str << _primitive->to_string();
        return str.str();
    }
    if (isFunction()) {
        return _function->to_string();
    }
    return "unknown type";
}

} // namespace type

