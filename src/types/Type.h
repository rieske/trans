#ifndef _TYPE_H_
#define _TYPE_H_

#include "Primitive.h"
#include "Function.h"

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <utility>

namespace type {

enum class Qualifier {
    CONST, VOLATILE
};

class Type {
public:
    struct Member {
        std::string name;
        // Stored by value after Type is complete enough via recursive optional vector.
        std::unique_ptr<Type> type;
        int offsetBytes { 0 };

        Member() = default;
        Member(std::string n, Type t, int off);
        Member(const Member& other);
        Member& operator=(const Member& other);
        Member(Member&&) noexcept = default;
        Member& operator=(Member&&) noexcept = default;
    };

    friend Type voidType();
    friend Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers);
    friend Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers);
    friend Type function(const Type& returnType, const std::vector<Type>& arguments);
    friend Type structure(std::vector<std::pair<std::string, Type>> members);

    int getSize() const;
    bool canAssignFrom(const Type& other) const;

    bool isVoid() const;
    bool isPrimitive() const;
    Primitive getPrimitive() const;
    bool isPointer() const;
    bool isFunction() const;
    Function getFunction() const;
    bool isStructure() const;
    const std::vector<Member>& getStructMembers() const;
    bool memberOffset(const std::string& memberName, int& offsetBytes) const;
    bool memberType(const std::string& memberName, Type& outType) const;

    bool isConst() const;
    bool isVolatile() const;

    Type dereference() const;

    std::string to_string() const;

private:
    Type(std::vector<Qualifier> qualifiers);
    Type(const Primitive& primitive, std::vector<Qualifier> qualifiers);
    Type(const Type& returnType, const std::vector<Type>& arguments);

    std::optional<Primitive> _primitive;
    std::optional<Function> _function;
    std::optional<std::vector<Member>> _structure;

    int _size {0};
    bool _const {false};
    bool _volatile {false};

    int _indirection {0};
};

Type voidType();
Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers = {});
Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers = {});
Type function(const Type& returnType, const std::vector<Type>& arguments = {});
Type structure(std::vector<std::pair<std::string, Type>> members);

Type signedCharacter(const std::vector<Qualifier>& qualifiers = {});
Type unsignedCharacter(const std::vector<Qualifier>& qualifiers = {});
Type signedInteger(const std::vector<Qualifier>& qualifiers = {});
Type unsignedInteger(const std::vector<Qualifier>& qualifiers = {});
Type signedLong(const std::vector<Qualifier>& qualifiers = {});
Type unsignedLong(const std::vector<Qualifier>& qualifiers = {});

Type floating(const std::vector<Qualifier>& qualifiers = {});
Type doubleFloating(const std::vector<Qualifier>& qualifiers = {});
Type longDoubleFloating(const std::vector<Qualifier>& qualifiers = {});

} // namespace type

#endif // _TYPE_H
