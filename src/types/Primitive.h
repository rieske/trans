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
    SignedInt128,
    UnsignedInt128,
    Boolean,
    Float,
    Double,
    LongDouble,
    ComplexFloat,
    ComplexDouble,
    ComplexLongDouble
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
    static Primitive signedInt128();
    static Primitive unsignedInt128();
    static Primitive boolean();

    static Primitive floating();
    static Primitive doubleFloating();
    static Primitive longDoubleFloating();
    static Primitive complexFloat();
    static Primitive complexDouble();
    static Primitive complexLongDouble();

    PrimitiveKind kind() const;
    int getSize() const;
    int getAlignment() const;
    bool isSigned() const;
    bool isFloating() const;
    bool isComplex() const;
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
