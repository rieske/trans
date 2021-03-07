#include "Primitive.h"

#include <stdexcept>

namespace type {

Primitive Primitive::signedCharacter() {
    return Primitive{1, true, false};
}

Primitive Primitive::unsignedCharacter() {
    return Primitive{1, false, false};
}

Primitive Primitive::signedInteger() {
    return Primitive{4, true, false};
}

Primitive Primitive::unsignedInteger() {
    return Primitive{4, false, false};
}

Primitive Primitive::signedLong() {
    return Primitive{8, true, false};
}

Primitive Primitive::unsignedLong() {
    return Primitive{8, false, false};
}

Primitive Primitive::floating() {
    return Primitive{4, true, true};
}

Primitive Primitive::doubleFloating() {
    return Primitive{8, true, true};
}

Primitive Primitive::longDoubleFloating() {
    return Primitive{16, true, true};
}

Primitive::Primitive(int _size, bool _signed, bool _float):
    _size{_size},
    _signed{_signed},
    _float{_float}
{
}

int Primitive::getSize() const {
    return _size;
}

bool Primitive::isSigned() const {
    return _signed;
}

bool Primitive::isFloating() const {
    return _float;
}

std::string Primitive::base_primitive_string() const {
    switch (_size) {
        case 1:
            return "char";
        case 4:
            return isFloating() ? "float" : "int";
        case 8:
            return isFloating() ? "double" : "long";
        case 16:
            return "long double";
        default:
            throw std::runtime_error{"unknown primitive with size " + std::to_string(_size)};
    }
}

std::string Primitive::to_string() const {
    if (isSigned()) {
        return base_primitive_string();
    }
    return "unsigned " + base_primitive_string();
}

} // namespace type

