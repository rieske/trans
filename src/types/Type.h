#ifndef _TYPE_H_
#define _TYPE_H_

#include "Primitive.h"
#include "Function.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace type {

enum class Qualifier {
    CONST, VOLATILE,
    // Parsed (C99 restrict / __restrict) and discarded; not stored on Type.
    RESTRICT
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

struct BitField {
    int width { 0 };
    int shift { 0 };
    bool isSigned { false };
};

// Mask for the low `width` bits of a bit-field (0..64).
inline unsigned long long bitFieldMask(int width) {
    if (width <= 0) {
        return 0ull;
    }
    if (width >= 64) {
        return ~0ull;
    }
    return (1ull << width) - 1ull;
}

struct MemberSpec;

class Type {
public:
    struct Member {
        std::string name;
        std::unique_ptr<Type> type;
        int offsetBytes { 0 };
        std::optional<BitField> bitField;

        Member() = default;
        Member(std::string n, Type t, int off, std::optional<BitField> bits = {});
        Member(const Member& other);
        Member& operator=(const Member& other);
        Member(Member&&) noexcept = default;
        Member& operator=(Member&&) noexcept = default;

        bool isBitField() const { return bitField.has_value(); }
    };

    // Shared layout for struct and union (union: all members at offset 0).
    struct StructBody {
        std::vector<Member> members;
        int size { 0 };
        bool complete { false };
        // True for union types. Designated-init must not zero inactive union arms.
        bool isUnion { false };
        // GNU __attribute__((transparent_union)): assign/call accept member types.
        bool transparentUnion { false };
        // GNU __attribute__((packed)): no member/tail padding; alignment 1.
        bool packed { false };
    };

    friend Type voidType();
    friend Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers);
    friend Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers);
    friend Type function(const Type& returnType, const std::vector<Type>& arguments, bool variadic);
    friend Type array(const Type& elementType, int elementCount);
    friend Type incompleteArray(const Type& elementType);
    friend Type incompleteRecord();
    friend Type structure(const std::vector<std::pair<std::string, Type>>& members);
    friend void completeStructure(Type& structType, const std::vector<MemberSpec>& members,
            bool packed);
    friend Type unionType(const std::vector<std::pair<std::string, Type>>& members);
    friend void completeUnion(Type& unionType, const std::vector<MemberSpec>& members, bool packed);

    int getSize() const;
    // Natural alignment in bytes (SysV/amd64 stand-in).
    int getAlignment() const;
    bool canAssignFrom(const Type& other) const;

    // Structural equality ignoring const/volatile at every level.
    // Records compare by structureBodyIdentity(); pointers peel via dereference().
    bool equivalentTo(const Type& other) const;
    // Same shape as equivalentTo, but const/volatile must match at every level.
    // Not C 6.2.7 compatibility (e.g. int[] vs int[3] is false here).
    bool sameQualifiedType(const Type& other) const;
    // Drop top-level const/volatile, then sameQualifiedType.
    bool sameUnqualifiedType(const Type& other) const;
    // C 6.2.7 / 6.7.6.2: compatible types. int[] and int[3] are compatible;
    // int[2] and int[3] are not. Qualifiers must match at each level.
    bool compatibleWith(const Type& other) const;
    // Composite of two compatible types (C 6.2.7). Empty if not compatible.
    std::optional<Type> composite(const Type& other) const;

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
    bool isIncompleteArray() const;
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
    // GNU transparent_union attribute on a complete union typedef.
    bool isTransparentUnion() const;
    void markTransparentUnion();
    bool isPacked() const;
    // Relayout a complete record with alignment 1, or mark an incomplete one.
    void applyPacked();
    // Array, struct, or union (brace-init / multi-word aggregates).
    bool isAggregate() const;
    // Complete struct or union layout.
    bool isCompleteRecord() const;
    // Complete C struct only (not unions). Prefer isCompleteRecord for both kinds.
    bool isCompleteStructure() const { return isStructure() && isCompleteRecord(); }

    // Record members (struct or union).
    const std::vector<Member>& getMembers() const;
    int memberCount() const;

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
    // Add top-level const/volatile onto a copy. restrict is ignored.
    Type withQualifiers(const std::vector<Qualifier>& qualifiers) const;

    Type dereference() const;
    // Type of *p or a[i]. Empty if this is not a pointer or array.
    std::optional<Type> indexElement() const;

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
        bool complete { true };
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
    void applyQualifiers(const std::vector<Qualifier>& qualifiers);

    Payload _payload { VoidPayload{} };
    bool _const { false };
    bool _volatile { false };
};

Type voidType();
Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers = {});
Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers = {});
Type function(const Type& returnType, const std::vector<Type>& arguments = {}, bool variadic = false);
Type array(const Type& elementType, int elementCount);
Type incompleteArray(const Type& elementType);
// Incomplete record tag (struct or union not yet known). Both live as RecordPayload;
// kind() is Struct vs Union via shared StructBody::isUnion once completed.
// Pointers and aliases that share structureBodyIdentity() see the same body when
// completeStructure/completeUnion mutates it - required for self-referential tags.
Type incompleteRecord();
// Compatibility alias for incompleteRecord().
inline Type incompleteStructure() { return incompleteRecord(); }
BitField makeBitField(const Type& declared, int width, int shift);

struct FoundMember {
    std::string name;
    Type type { voidType() };
    int offsetBytes { 0 };
    std::optional<BitField> bitField;

    bool isBitField() const { return bitField.has_value(); }

    FoundMember atBase(int baseOffset) const {
        FoundMember copy = *this;
        copy.offsetBytes += baseOffset;
        return copy;
    }
};

struct MemberPath {
    FoundMember member;
    std::vector<int> indices;
};

std::optional<MemberPath> lookupMemberPath(const Type& record, const std::string& name);
std::optional<FoundMember> lookupMember(const Type& record, const std::string& name);
std::optional<FoundMember> memberAt(const Type& record, int index);

// __builtin_offsetof: byte offset of a named non-bit-field member of a complete record.
enum class OffsetofStatus {
    Ok,
    Incomplete,
    Missing,
    BitField,
};

struct OffsetofResult {
    OffsetofStatus status { OffsetofStatus::Missing };
    int offsetBytes { 0 };
};

OffsetofResult resolveOffsetof(const Type& record, const std::string& name);

struct MemberSpec {
    std::string name;
    Type type;
    // nullopt: ordinary member. 0: zero-width (unnamed) bit-field. >0: named width.
    std::optional<int> bitWidth;

    MemberSpec(std::string n, Type t, std::optional<int> width = std::nullopt) :
            name { std::move(n) }, type { std::move(t) }, bitWidth { width } {}
};

// Ordinary members only (no bit-fields). Prefer completeStructure(MemberSpec) for bit-fields.
Type structure(const std::vector<std::pair<std::string, Type>>& members = {});
// Completes a shared StructBody as a struct (isUnion=false). All Type values
// holding that body identity update kind()/layout together.
void completeStructure(Type& structType, const std::vector<MemberSpec>& members,
        bool packed = false);
// Ordinary members only (no bit-fields). Prefer completeUnion(MemberSpec) for bit-fields.
Type unionType(const std::vector<std::pair<std::string, Type>>& members = {});
// Union: all members at offset 0; size is the max member stride. packed: alignment 1.
void completeUnion(Type& unionType, const std::vector<MemberSpec>& members,
        bool packed = false);

Type signedCharacter(const std::vector<Qualifier>& qualifiers = {});
Type unsignedCharacter(const std::vector<Qualifier>& qualifiers = {});
Type boolean(const std::vector<Qualifier>& qualifiers = {});
Type signedShort(const std::vector<Qualifier>& qualifiers = {});
Type unsignedShort(const std::vector<Qualifier>& qualifiers = {});
Type signedInteger(const std::vector<Qualifier>& qualifiers = {});
Type unsignedInteger(const std::vector<Qualifier>& qualifiers = {});
Type signedLong(const std::vector<Qualifier>& qualifiers = {});
Type unsignedLong(const std::vector<Qualifier>& qualifiers = {});
Type signedInt128(const std::vector<Qualifier>& qualifiers = {});
Type unsignedInt128(const std::vector<Qualifier>& qualifiers = {});

// GCC/SysV enum policy: smallest of {signed int, unsigned int, signed long}
// that covers [minValue, maxValue]. A single constant is the degenerate range.
Type enumUnderlyingType(long minValue, long maxValue);

Type floating(const std::vector<Qualifier>& qualifiers = {});
Type doubleFloating(const std::vector<Qualifier>& qualifiers = {});
Type longDoubleFloating(const std::vector<Qualifier>& qualifiers = {});
Type complexFloat(const std::vector<Qualifier>& qualifiers = {});
Type complexDouble(const std::vector<Qualifier>& qualifiers = {});
Type complexLongDouble(const std::vector<Qualifier>& qualifiers = {});

// SysV AMD64 va_list: array-of-1 of {unsigned gp_offset, unsigned fp_offset,
// void *overflow_arg_area, void *reg_save_area}. sizeof == 24.
Type builtinVaListTagType();
Type builtinVaListType();

} // namespace type

#endif // _TYPE_H
