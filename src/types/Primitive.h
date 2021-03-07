#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include <string>

namespace type {

class Primitive {
public:
    static Primitive signedCharacter();
    static Primitive unsignedCharacter();
    static Primitive signedInteger();
    static Primitive unsignedInteger();
    static Primitive signedLong();
    static Primitive unsignedLong();

    static Primitive floating();
    static Primitive doubleFloating();
    static Primitive longDoubleFloating();


    int getSize() const;
    bool isSigned() const;
    bool isFloating() const;

    bool canAssignFrom(const Primitive& other) const;

    std::string to_string() const;

private:
    Primitive(int _size, bool _signed, bool _float);

    std::string base_primitive_string() const;

    int _size;
    bool _signed;
    bool _float;
};

} // namespace type

#endif // _PRIMITIVE_H_
