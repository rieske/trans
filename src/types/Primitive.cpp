#include "Primitive.h"

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

} // namespace type

