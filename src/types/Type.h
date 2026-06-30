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
        std::unique_ptr<Type> type;
        int offsetBytes { 0 };

        Member() = default;
        Member(std::string n, Type t, int off);
        Member(const Member& other);
        Member& operator=(const Member& other);
        Member(Member&&) noexcept = default;
        Member& operator=(Member&&) noexcept = default;
    };

    struct StructBody {
        std::vector<Member> members;
        int size { 0 };
        bool complete { false };
    };

    friend Type voidType();
    friend Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers);
    friend Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers);
    friend Type function(const Type& returnType, const std::vector<Type>& arguments);
    friend Type incompleteStructure();
    friend Type structure(std::vector<std::pair<std::string, Type>> members);
    friend void completeStructure(Type& structType, std::vector<std::pair<std::string, Type>> members);

    int getSize() const;
    bool canAssignFrom(const Type& other) const;

    bool isVoid() const;
    bool isPrimitive() const;
    Primitive getPrimitive() const;
    bool isPointer() const;
    bool isFunction() const;
    Function getFunction() const;
    bool isStructure() const;
    bool isCompleteStructure() const;
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
    // Shared so struct tags and pointers to that struct see the same layout when completed.
    std::shared_ptr<StructBody> _structure;

    int _size {0};
    bool _const {false};
    bool _volatile {false};

    int _indirection {0};
};

Type voidType();
Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers = {});
Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers = {});
Type function(const Type& returnType, const std::vector<Type>& arguments = {});
// Incomplete struct (for tags / self-referential pointers); complete with completeStructure.
Type incompleteStructure();
Type structure(std::vector<std::pair<std::string, Type>> members);
void completeStructure(Type& structType, std::vector<std::pair<std::string, Type>> members);

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
