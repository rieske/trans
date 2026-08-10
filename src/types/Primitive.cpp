#include "Primitive.h"

#include <stdexcept>

namespace type {

Primitive Primitive::signedCharacter() {
    return Primitive { PrimitiveKind::SignedChar };
}

Primitive Primitive::unsignedCharacter() {
    return Primitive { PrimitiveKind::UnsignedChar };
}

Primitive Primitive::signedShort() {
    return Primitive { PrimitiveKind::SignedShort };
}

Primitive Primitive::unsignedShort() {
    return Primitive { PrimitiveKind::UnsignedShort };
}

Primitive Primitive::signedInteger() {
    return Primitive { PrimitiveKind::SignedInteger };
}

Primitive Primitive::unsignedInteger() {
    return Primitive { PrimitiveKind::UnsignedInteger };
}

Primitive Primitive::signedLong() {
    return Primitive { PrimitiveKind::SignedLong };
}

Primitive Primitive::unsignedLong() {
    return Primitive { PrimitiveKind::UnsignedLong };
}

Primitive Primitive::boolean() {
    return Primitive { PrimitiveKind::Boolean };
}

Primitive Primitive::floating() {
    return Primitive { PrimitiveKind::Float };
}

Primitive Primitive::doubleFloating() {
    return Primitive { PrimitiveKind::Double };
}

Primitive Primitive::longDoubleFloating() {
    return Primitive { PrimitiveKind::LongDouble };
}

Primitive::Primitive(PrimitiveKind kind) :
        kind_ { kind } {
}

PrimitiveKind Primitive::kind() const {
    return kind_;
}

int Primitive::getSize() const {
    switch (kind_) {
    case PrimitiveKind::SignedChar:
    case PrimitiveKind::UnsignedChar:
    case PrimitiveKind::Boolean:
        return 1;
    case PrimitiveKind::SignedShort:
    case PrimitiveKind::UnsignedShort:
        return 2;
    case PrimitiveKind::SignedInteger:
    case PrimitiveKind::UnsignedInteger:
    case PrimitiveKind::Float:
        return 4;
    case PrimitiveKind::SignedLong:
    case PrimitiveKind::UnsignedLong:
    case PrimitiveKind::Double:
        return 8;
    case PrimitiveKind::LongDouble:
        return 16;
    }
    throw std::runtime_error { "unknown primitive kind" };
}

bool Primitive::isSigned() const {
    switch (kind_) {
    case PrimitiveKind::UnsignedChar:
    case PrimitiveKind::UnsignedShort:
    case PrimitiveKind::UnsignedInteger:
    case PrimitiveKind::UnsignedLong:
    case PrimitiveKind::Boolean:
        return false;
    case PrimitiveKind::SignedChar:
    case PrimitiveKind::SignedShort:
    case PrimitiveKind::SignedInteger:
    case PrimitiveKind::SignedLong:
    case PrimitiveKind::Float:
    case PrimitiveKind::Double:
    case PrimitiveKind::LongDouble:
        return true;
    }
    throw std::runtime_error { "unknown primitive kind" };
}

bool Primitive::isFloating() const {
    return kind_ == PrimitiveKind::Float
            || kind_ == PrimitiveKind::Double
            || kind_ == PrimitiveKind::LongDouble;
}

bool Primitive::isBoolean() const {
    return kind_ == PrimitiveKind::Boolean;
}

bool Primitive::isCharacter() const {
    return kind_ == PrimitiveKind::SignedChar || kind_ == PrimitiveKind::UnsignedChar;
}

bool Primitive::equivalentTo(const Primitive& other) const {
    return kind_ == other.kind_;
}

std::string Primitive::to_string() const {
    switch (kind_) {
    case PrimitiveKind::SignedChar:
        return "char";
    case PrimitiveKind::UnsignedChar:
        return "unsigned char";
    case PrimitiveKind::SignedShort:
        return "short";
    case PrimitiveKind::UnsignedShort:
        return "unsigned short";
    case PrimitiveKind::SignedInteger:
        return "int";
    case PrimitiveKind::UnsignedInteger:
        return "unsigned int";
    case PrimitiveKind::SignedLong:
        return "long";
    case PrimitiveKind::UnsignedLong:
        return "unsigned long";
    case PrimitiveKind::Boolean:
        return "bool";
    case PrimitiveKind::Float:
        return "float";
    case PrimitiveKind::Double:
        return "double";
    case PrimitiveKind::LongDouble:
        return "long double";
    }
    throw std::runtime_error { "unknown primitive kind" };
}

} // namespace type

