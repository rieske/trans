#ifndef _TYPE_H_
#define _TYPE_H_

#include "Primitive.h"
#include "Function.h"
#include "TypeQualifier.h"

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace type {

class Type {
public:
    friend Type voidType();
    friend Type primitive(const Primitive& primitive, const std::vector<TypeQualifier>& qualifiers);
    friend Type pointer(const Type& pointsTo, const std::vector<TypeQualifier>& qualifiers);
    friend Type function(const Type& returnType, const std::vector<Type>& arguments);
    friend Type structure(const std::vector<Type>& members);

    int getSize() const;
    bool canAssignFrom(const Type& other) const;

    bool isVoid() const;
    bool isPrimitive() const;
    Primitive getPrimitive() const;
    bool isPointer() const;
    bool isFunction() const;
    Function getFunction() const;
    bool isStructure() const;

    bool isConst() const;
    bool isVolatile() const;

    Type dereference() const;

    std::string to_string() const;

private:
    Type(std::vector<TypeQualifier> qualifiers);
    Type(const Primitive& primitive, std::vector<TypeQualifier> qualifiers);
    Type(const Type& returnType, const std::vector<Type>& arguments);

    std::optional<Primitive> _primitive;
    std::optional<Function> _function;

    int _size {0};
    bool _const {false};
    bool _volatile {false};

    int _indirection {0};
};

Type voidType();
Type primitive(const Primitive& primitive, const std::vector<TypeQualifier>& qualifiers = {});
Type pointer(const Type& pointsTo, const std::vector<TypeQualifier>& qualifiers = {});
Type function(const Type& returnType, const std::vector<Type>& arguments = {});
Type structure(const std::vector<Type>& members = {});

Type signedCharacter(const std::vector<TypeQualifier>& qualifiers = {});
Type unsignedCharacter(const std::vector<TypeQualifier>& qualifiers = {});
Type signedInteger(const std::vector<TypeQualifier>& qualifiers = {});
Type unsignedInteger(const std::vector<TypeQualifier>& qualifiers = {});
Type signedLong(const std::vector<TypeQualifier>& qualifiers = {});
Type unsignedLong(const std::vector<TypeQualifier>& qualifiers = {});

Type floating(const std::vector<TypeQualifier>& qualifiers = {});
Type doubleFloating(const std::vector<TypeQualifier>& qualifiers = {});
Type longDoubleFloating(const std::vector<TypeQualifier>& qualifiers = {});

} // namespace type

#endif // _TYPE_H
