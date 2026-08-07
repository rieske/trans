#ifndef _TYPE_H_
#define _TYPE_H_

#include "Primitive.h"
#include "Function.h"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace type {

enum class Qualifier {
    CONST, VOLATILE
};

// Kind of this Type node. Pointers and arrays are recursive: they hold a
// shared_ptr to the pointee/element Type (not an indirection counter on a
// flattened payload).
enum class TypeKind {
    Void,
    Primitive,
    Pointer,
    Function,
    Array,
    Struct,
    Union
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

    // Shared layout for struct and union (union: all members at offset 0).
    struct StructBody {
        std::vector<Member> members;
        int size { 0 };
        bool complete { false };
        // True for union types. Designated-init must not zero inactive union arms.
        bool isUnion { false };
    };

    friend Type voidType();
    friend Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers);
    friend Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers);
    friend Type function(const Type& returnType, const std::vector<Type>& arguments, bool variadic);
    friend Type array(const Type& elementType, int elementCount);
    friend Type incompleteRecord();
    friend Type structure(const std::vector<std::pair<std::string, Type>>& members);
    friend void completeStructure(Type& structType,
            const std::vector<std::pair<std::string, Type>>& members);
    friend Type unionType(const std::vector<std::pair<std::string, Type>>& members);
    friend void completeUnion(Type& unionType,
            const std::vector<std::pair<std::string, Type>>& members);

    int getSize() const;
    // Natural alignment in bytes (SysV/amd64 stand-in).
    int getAlignment() const;
    bool canAssignFrom(const Type& other) const;

    // Structural equality ignoring top-level const/volatile on both sides.
    // Records compare by structureBodyIdentity(); pointers peel via dereference().
    bool equivalentTo(const Type& other) const;

    TypeKind kind() const;

    bool isVoid() const;
    bool isPrimitive() const;
    // Throws std::domain_error unless kind is Primitive.
    Primitive getPrimitive() const;
    bool isPointer() const;
    bool isFunction() const;
    // Throws std::domain_error unless kind is Function.
    Function getFunction() const;
    bool isArray() const;
    Type getElementType() const;
    int getArraySize() const;
    // Parameter arrays decay to pointer-to-element.
    Type decayArray() const;
    // Storage stride of one element (0 allowed for empty complete records).
    int getElementStride() const;

    // Struct or union (has member layout at this type; not through a pointer).
    bool isRecord() const;
    // C struct only - not a union.
    bool isStructure() const;
    bool isUnion() const;
    // Array, struct, or union (brace-init / multi-word aggregates).
    bool isAggregate() const;
    // Complete struct or union layout.
    bool isCompleteRecord() const;
    // Complete C struct only (not unions). Prefer isCompleteRecord for both kinds.
    bool isCompleteStructure() const { return isStructure() && isCompleteRecord(); }

    // Record members (struct or union). getStructMembers is a compatibility alias.
    const std::vector<Member>& getMembers() const;
    const std::vector<Member>& getStructMembers() const { return getMembers(); }
    bool memberOffset(const std::string& memberName, int& offsetBytes) const;
    bool memberType(const std::string& memberName, Type& outType) const;
    // Indexed access for positional initializers (structure/union).
    int memberCount() const;
    bool memberAt(int index, std::string& name, Type& outType, int& offsetBytes) const;

    // True when this is a record with no completed layout yet.
    bool isIncompleteRecord() const;
    // Incomplete C struct only (not unions). Prefer isIncompleteRecord for both kinds.
    bool isIncompleteStructure() const { return isStructure() && isIncompleteRecord(); }
    // Mutate the shared member layout in place so existing Type copies of an
    // incomplete tag (e.g. struct Node *next) observe the completed layout.
    void completeStructure(const std::vector<std::pair<std::string, Type>>& members);

    bool isConst() const;
    bool isVolatile() const;
    // Drop top-level const/volatile (C ignores them on function parameters).
    Type withoutTopLevelQualifiers() const;

    Type dereference() const;

    // Shared StructBody address for identity (struct member fixups, tag aliases).
    const void* structureBodyIdentity() const;

    std::string to_string() const;

private:
    // Closed sum: exactly one arm active (std::variant). Qualifiers are orthogonal.
    struct VoidPayload {};
    struct PrimitivePayload { Primitive value; };
    struct PointerPayload { std::shared_ptr<Type> pointee; };
    struct FunctionPayload { Function value; };
    struct ArrayPayload {
        std::shared_ptr<Type> element;
        int count { 0 };
        int sizeBytes { 0 };
    };
    struct RecordPayload {
        // Shared so tags and pointers to that record see the same layout when completed.
        std::shared_ptr<StructBody> body;
    };
    using Payload = std::variant<
            VoidPayload,
            PrimitivePayload,
            PointerPayload,
            FunctionPayload,
            ArrayPayload,
            RecordPayload>;

    const RecordPayload* recordPayload() const;
    RecordPayload* recordPayload();
    const StructBody* body() const;
    StructBody* body();
    const ArrayPayload* arrayPayload() const;

    Type(std::vector<Qualifier> qualifiers);
    Type(const Primitive& primitive, std::vector<Qualifier> qualifiers);
    Type(const Type& returnType, const std::vector<Type>& arguments, bool variadic = false);

    Payload _payload { VoidPayload{} };
    bool _const { false };
    bool _volatile { false };
};

Type voidType();
Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers = {});
Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers = {});
Type function(const Type& returnType, const std::vector<Type>& arguments = {}, bool variadic = false);
Type array(const Type& elementType, int elementCount);
// Incomplete record tag (struct or union not yet known). Both live as RecordPayload;
// kind() is Struct vs Union via shared StructBody::isUnion once completed.
// Pointers and aliases that share structureBodyIdentity() see the same body when
// completeStructure/completeUnion mutates it - required for self-referential tags.
Type incompleteRecord();
// Compatibility alias for incompleteRecord().
inline Type incompleteStructure() { return incompleteRecord(); }
Type structure(const std::vector<std::pair<std::string, Type>>& members = {});
// Completes a shared StructBody as a struct (isUnion=false). All Type values
// holding that body identity update kind()/layout together.
void completeStructure(Type& structType,
        const std::vector<std::pair<std::string, Type>>& members);
// Union: all members at offset 0; size is the max member stride.
Type unionType(const std::vector<std::pair<std::string, Type>>& members = {});
void completeUnion(Type& unionType,
        const std::vector<std::pair<std::string, Type>>& members);

Type signedCharacter(const std::vector<Qualifier>& qualifiers = {});
Type unsignedCharacter(const std::vector<Qualifier>& qualifiers = {});
Type signedShort(const std::vector<Qualifier>& qualifiers = {});
Type unsignedShort(const std::vector<Qualifier>& qualifiers = {});
Type signedInteger(const std::vector<Qualifier>& qualifiers = {});
Type unsignedInteger(const std::vector<Qualifier>& qualifiers = {});
Type signedLong(const std::vector<Qualifier>& qualifiers = {});
Type unsignedLong(const std::vector<Qualifier>& qualifiers = {});

Type floating(const std::vector<Qualifier>& qualifiers = {});
Type doubleFloating(const std::vector<Qualifier>& qualifiers = {});
Type longDoubleFloating(const std::vector<Qualifier>& qualifiers = {});

// SysV AMD64 va_list: array-of-1 of {unsigned gp_offset, unsigned fp_offset,
// void *overflow_arg_area, void *reg_save_area}. sizeof == 24.
Type builtinVaListTagType();
Type builtinVaListType();

} // namespace type

#endif // _TYPE_H
