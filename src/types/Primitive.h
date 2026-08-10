#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include <string>

namespace type {

enum class PrimitiveKind {
    SignedChar,
    UnsignedChar,
    SignedShort,
    UnsignedShort,
    SignedInteger,
    UnsignedInteger,
    SignedLong,
    UnsignedLong,
    Boolean,
    Float,
    Double,
    LongDouble
};

class Primitive {
public:
    static Primitive signedCharacter();
    static Primitive unsignedCharacter();
    static Primitive signedShort();
    static Primitive unsignedShort();
    static Primitive signedInteger();
    static Primitive unsignedInteger();
    static Primitive signedLong();
    static Primitive unsignedLong();
    static Primitive boolean();

    static Primitive floating();
    static Primitive doubleFloating();
    static Primitive longDoubleFloating();

    PrimitiveKind kind() const;
    int getSize() const;
    bool isSigned() const;
    bool isFloating() const;
    bool isBoolean() const;
    bool isCharacter() const;

    bool equivalentTo(const Primitive& other) const;

    std::string to_string() const;

private:
    explicit Primitive(PrimitiveKind kind);

    PrimitiveKind kind_;
};

} // namespace type

#endif // _PRIMITIVE_H_
