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

Primitive Primitive::signedInt128() {
    return Primitive { PrimitiveKind::SignedInt128 };
}

Primitive Primitive::unsignedInt128() {
    return Primitive { PrimitiveKind::UnsignedInt128 };
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

Primitive Primitive::complexFloat() {
    return Primitive { PrimitiveKind::ComplexFloat };
}

Primitive Primitive::complexDouble() {
    return Primitive { PrimitiveKind::ComplexDouble };
}

Primitive Primitive::complexLongDouble() {
    return Primitive { PrimitiveKind::ComplexLongDouble };
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
    case PrimitiveKind::ComplexFloat:
        return 8;
    case PrimitiveKind::SignedInt128:
    case PrimitiveKind::UnsignedInt128:
    case PrimitiveKind::LongDouble:
    case PrimitiveKind::ComplexDouble:
        return 16;
    case PrimitiveKind::ComplexLongDouble:
        return 32;
    }
    throw std::runtime_error { "unknown primitive kind" };
}

int Primitive::getAlignment() const {
    switch (kind_) {
    case PrimitiveKind::ComplexFloat:
        return 4;
    case PrimitiveKind::ComplexDouble:
        return 8;
    case PrimitiveKind::ComplexLongDouble:
        return 16;
    default:
        return getSize();
    }
}

bool Primitive::isSigned() const {
    switch (kind_) {
    case PrimitiveKind::UnsignedChar:
    case PrimitiveKind::UnsignedShort:
    case PrimitiveKind::UnsignedInteger:
    case PrimitiveKind::UnsignedLong:
    case PrimitiveKind::UnsignedInt128:
    case PrimitiveKind::Boolean:
        return false;
    case PrimitiveKind::SignedChar:
    case PrimitiveKind::SignedShort:
    case PrimitiveKind::SignedInteger:
    case PrimitiveKind::SignedLong:
    case PrimitiveKind::SignedInt128:
    case PrimitiveKind::Float:
    case PrimitiveKind::Double:
    case PrimitiveKind::LongDouble:
    case PrimitiveKind::ComplexFloat:
    case PrimitiveKind::ComplexDouble:
    case PrimitiveKind::ComplexLongDouble:
        return true;
    }
    throw std::runtime_error { "unknown primitive kind" };
}

bool Primitive::isFloating() const {
    return kind_ == PrimitiveKind::Float
            || kind_ == PrimitiveKind::Double
            || kind_ == PrimitiveKind::LongDouble;
}

bool Primitive::isComplex() const {
    return kind_ == PrimitiveKind::ComplexFloat
            || kind_ == PrimitiveKind::ComplexDouble
            || kind_ == PrimitiveKind::ComplexLongDouble;
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
    case PrimitiveKind::SignedInt128:
        return "__int128";
    case PrimitiveKind::UnsignedInt128:
        return "unsigned __int128";
    case PrimitiveKind::Boolean:
        return "bool";
    case PrimitiveKind::Float:
        return "float";
    case PrimitiveKind::Double:
        return "double";
    case PrimitiveKind::LongDouble:
        return "long double";
    case PrimitiveKind::ComplexFloat:
        return "_Complex float";
    case PrimitiveKind::ComplexDouble:
        return "_Complex double";
    case PrimitiveKind::ComplexLongDouble:
        return "_Complex long double";
    }
    throw std::runtime_error { "unknown primitive kind" };
}

} // namespace type

