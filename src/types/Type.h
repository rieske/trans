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
    static Type voidType();
    static Type primitive(const Primitive& primitive, const std::vector<TypeQualifier>& qualifiers = {});
    static Type pointer(const Type& pointsTo, const std::vector<TypeQualifier>& qualifiers = {});
    static Type function(const Type& returnType, const std::vector<Type>& arguments = {});
    static Type structure(const std::vector<Type>& members = {});

    int getSize() const;
    bool canAssignFrom(const Type& other) const;

    bool isVoid() const;
    bool isPrimitive() const;
    Primitive getPrimitive() const;
    bool isPointer() const;
    bool isFunction() const;
    Type getReturnType() const;
    Type getArguments() const;
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

} // namespace type

#endif // _TYPE_H
