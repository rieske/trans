#ifndef _TYPE_H_
#define _TYPE_H_

#include "TypeQualifier.h"

#include <vector>

namespace type {

class Type {
public:
    static Type signedCharacter(std::vector<TypeQualifier> qualifiers = {});
    static Type unsignedCharacter(std::vector<TypeQualifier> qualifiers = {});
    static Type signedInteger(std::vector<TypeQualifier> qualifiers = {});
    static Type unsignedInteger(std::vector<TypeQualifier> qualifiers = {});
    static Type signedLong(std::vector<TypeQualifier> qualifiers = {});
    static Type unsignedLong(std::vector<TypeQualifier> qualifiers = {});

    static Type floating(std::vector<TypeQualifier> qualifiers = {});
    static Type doubleFloating(std::vector<TypeQualifier> qualifiers = {});
    static Type longDoubleFloating(std::vector<TypeQualifier> qualifiers = {});

    static Type pointer(const Type& pointsTo, std::vector<TypeQualifier> qualifiers = {});

    int getSize() const;
    bool isSigned() const;
    bool isFloating() const;
    bool isConst() const;
    bool isVolatile() const;

    bool canAssignFrom(const Type& other) const;

    Type dereference() const;

private:
    Type(int _size, bool _signed, bool _float, std::vector<TypeQualifier> qualifiers);

    int _size;
    bool _signed;
    bool _float;
    bool _const;
    bool _volatile;

    int _indirection {0};
};

} // namespace type

#endif // _TYPE_H
