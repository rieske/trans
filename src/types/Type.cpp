#include "Type.h"

#include <stdexcept>
#include <algorithm>
#include <sstream>

namespace type {

static const int POINTER_SIZE {8};

Type voidType() {
    return Type{{}};
}

Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers) {
    return Type{primitive, qualifiers};
}

Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers) {
    auto p = Type{pointsTo};
    p._indirection = pointsTo._indirection + 1;
    // Keep array/struct/function payload and pointee _size so dereference()
    // recovers the full type (needed for pointer-to-array: int (*)[3] has
    // pointee size 24, used as outer stride for p[i][j]).
    // isArray() is false while _indirection > 0; getSize() returns POINTER_SIZE.
    // Do not overwrite _size - that would break array size after dereference.
    for (const auto& q : qualifiers) {
        if (q == Qualifier::CONST) {
            p._const = true;
        } else if (q == Qualifier::VOLATILE) {
            p._volatile = true;
        }
    }
    return p;
}

Type function(const Type& returnType, const std::vector<Type>& arguments, bool variadic) {
    return Type{returnType, arguments, variadic};
}

namespace {
int alignedStride(const Type& elementType) {
    int size = elementType.getSize();
    if (size < 1) {
        size = 1;
    }
    // System V natural element size (int arrays are 4-byte stride so
    // SHA1_CTX.ihv[5] is 20 bytes and buffer[] lands at offset 28).
    return size;
}
} // namespace

Type array(const Type& elementType, int elementCount) {
    if (elementCount < 0) {
        throw std::invalid_argument { "array size must be non-negative" };
    }
    Type result { std::vector<Qualifier> {} };
    result._elementType = std::make_shared<Type>(elementType);
    result._arraySize = elementCount;
    result._size = alignedStride(elementType) * elementCount;
    return result;
}

Type signedCharacter(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedCharacter(), qualifiers);
}
Type unsignedCharacter(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedCharacter(), qualifiers);
}

Type signedShort(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedShort(), qualifiers);
}
Type unsignedShort(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedShort(), qualifiers);
}

Type signedInteger(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedInteger(), qualifiers);
}

Type unsignedInteger(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedInteger(), qualifiers);
}

Type signedLong(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedLong(), qualifiers);
}

Type unsignedLong(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedLong(), qualifiers);
}

Type floating(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::floating(), qualifiers);
}

Type doubleFloating(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::doubleFloating(), qualifiers);
}

Type longDoubleFloating(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::longDoubleFloating(), qualifiers);
}

Type::Type(std::vector<Qualifier> qualifiers) {
    for (const auto& qualifier: qualifiers) {
        switch(qualifier) {
            case Qualifier::CONST:
                this->_const = true;
                break;
            case Qualifier::VOLATILE:
                this->_volatile = true;
                break;
            default:
                throw std::invalid_argument{"Unsupported type qualifier"};
        }
    }
}

Type::Type(const Primitive& primitive, std::vector<Qualifier> qualifiers):
    Type{qualifiers}
{
    _primitive = primitive;
}

Type::Type(const Type& returnType, const std::vector<Type>& arguments, bool variadic) {
    std::vector<std::unique_ptr<Type>> args;
    for (const auto& arg : arguments) {
        args.push_back(std::make_unique<Type>(arg));
    }
    _function = Function{std::make_unique<Type>(returnType), std::move(args), variadic};
}

int Type::getSize() const {
    if (isPointer()) {
        return POINTER_SIZE;
    }
    if (isArray()) {
        return _size;
    }
    if (isPrimitive()) {
        return _primitive->getSize();
    }
    if (isRecord()) {
        return _structure->size;
    }
    return _size;
}

bool Type::canAssignFrom(const Type& other) const {
    // Destination is *this, source is other. Product rules are looser than ISO C
    // but reject clear mismatches (struct/scalar, different struct bodies, void)
    // so semantic typeCheck is no longer a no-op.
    //
    // Arrays decay on both sides: member-array expressions keep getType() as
    // T[N] for sizeof while their result symbol is already a pointer, and binary
    // typeCheck uses getType() (e.g. `p == s.buf`).
    Type dst = *this;
    Type src = other;
    if (dst.isArray()) {
        dst = dst.decayArray();
    }
    if (src.isArray()) {
        src = src.decayArray();
    }
    if (src.isFunction()) {
        // Function designator decays to pointer-to-function.
        src = pointer(src);
    }
    if (dst.isFunction()) {
        return false;
    }

    if (dst.isVoid() || src.isVoid()) {
        return false;
    }

    // Struct/union: same body identity for value assignment. Also allow
    // pointer ↔ struct/union: glibc types bind/accept sockaddr args as
    // __CONST_SOCKADDR_ARG transparent unions (a union of pointer members);
    // callers pass `struct sockaddr *`, which must remain accepted.
    if (dst.isRecord() || src.isRecord()) {
        if (dst.isRecord() && src.isRecord()) {
            return dst.structureBodyIdentity() == src.structureBodyIdentity();
        }
        if (dst.isPointer() || src.isPointer()) {
            return true;
        }
        // struct/union vs bare primitive (not via pointer): reject.
        return false;
    }

    // Scalars: primitives and pointers freely convert (usual arithmetic
    // conversions, null pointer constants, and pointer/integer mixes used by
    // binary typeCheck sites that pass operands asymmetrically).
    const bool dstScalar = dst.isPrimitive() || dst.isPointer();
    const bool srcScalar = src.isPrimitive() || src.isPointer();
    return dstScalar && srcScalar;
}

bool Type::equivalentTo(const Type& other) const {
    Type a = withoutTopLevelQualifiers();
    Type b = other.withoutTopLevelQualifiers();
    // Peel pointer layers (same depth and equivalent pointee).
    while (a.isPointer() || b.isPointer()) {
        if (!a.isPointer() || !b.isPointer()) {
            return false;
        }
        a = a.dereference();
        b = b.dereference();
        a = a.withoutTopLevelQualifiers();
        b = b.withoutTopLevelQualifiers();
    }
    if (a.kind() != b.kind()) {
        return false;
    }
    switch (a.kind()) {
    case TypeKind::Void:
        return true;
    case TypeKind::Primitive:
        return a.getPrimitive().equivalentTo(b.getPrimitive());
    case TypeKind::Array:
        return a.getArraySize() == b.getArraySize()
                && a.getElementType().equivalentTo(b.getElementType());
    case TypeKind::Function: {
        const Function fa = a.getFunction();
        const Function fb = b.getFunction();
        if (fa.isVariadic() != fb.isVariadic()) {
            return false;
        }
        if (!fa.getReturnType().equivalentTo(fb.getReturnType())) {
            return false;
        }
        const auto aa = fa.getArguments();
        const auto ba = fb.getArguments();
        if (aa.size() != ba.size()) {
            return false;
        }
        for (std::size_t i = 0; i < aa.size(); ++i) {
            if (!aa[i].equivalentTo(ba[i])) {
                return false;
            }
        }
        return true;
    }
    case TypeKind::Struct:
    case TypeKind::Union:
        return a.structureBodyIdentity() == b.structureBodyIdentity();
    case TypeKind::Pointer:
        // Handled by peel loop above.
        return false;
    }
    return false;
}

TypeKind Type::kind() const {
    if (isPointer()) {
        return TypeKind::Pointer;
    }
    if (isArray()) {
        return TypeKind::Array;
    }
    if (isPrimitive()) {
        return TypeKind::Primitive;
    }
    if (_function.has_value()) {
        return TypeKind::Function;
    }
    if (_structure) {
        return _structure->isUnion ? TypeKind::Union : TypeKind::Struct;
    }
    return TypeKind::Void;
}

bool Type::isVoid() const {
    return kind() == TypeKind::Void;
}

bool Type::isPrimitive() const {
    return _primitive.has_value();
}

Primitive Type::getPrimitive() const {
    return *_primitive;
}

bool Type::isConst() const {
    return _const;
}

bool Type::isVolatile() const {
    return _volatile;
}

Type Type::withoutTopLevelQualifiers() const {
    Type t { *this };
    t._const = false;
    t._volatile = false;
    return t;
}

bool Type::isPointer() const {
    return _indirection;
}

bool Type::isFunction() const {
    // Pointer-to-function carries _function payload but is a pointer type, not a function type.
    return _function.has_value() && !isPointer();
}

Function Type::getFunction() const {
    return *_function;
}

bool Type::isArray() const {
    return _elementType != nullptr && _indirection == 0;
}

Type Type::getElementType() const {
    if (!isArray()) {
        throw std::domain_error { "not an array type" };
    }
    return *_elementType;
}

int Type::getArraySize() const {
    if (!isArray()) {
        throw std::domain_error { "not an array type" };
    }
    return _arraySize;
}

Type Type::decayArray() const {
    if (!isArray()) {
        throw std::domain_error { "not an array type" };
    }
    return pointer(getElementType());
}

int Type::getElementStride() const {
    if (!isArray()) {
        throw std::domain_error { "not an array type" };
    }
    return alignedStride(*_elementType);
}

Type Type::dereference() const {
    if (!isPointer()) {
        throw std::domain_error{"can not dereference non-pointer type"};
    }
    Type pointsTo = Type{*this};
    --pointsTo._indirection;
    return pointsTo;
}

std::string Type::to_string() const {
    if (isVoid()) {
        return "void";
    }
    if (isPointer()) {
        return dereference().to_string() + "*";
    }
    if (isArray()) {
        return getElementType().to_string() + "[" + std::to_string(_arraySize) + "]";
    }
    if (isPrimitive()) {
        std::stringstream str;
        if (isConst()) {
            str << "const ";
        }
        if (isVolatile()) {
            str << "volatile ";
        }
        str << _primitive->to_string();
        return str.str();
    }
    if (isFunction()) {
        return _function->to_string();
    }
    if (isUnion()) {
        return "union";
    }
    if (isStructure() || isRecord()) {
        return "struct";
    }
    return "unknown type";
}


Type::Member::Member(std::string n, Type t, int off) :
        name { std::move(n) },
        type { std::make_unique<Type>(std::move(t)) },
        offsetBytes { off }
{
}

Type::Member::Member(const Member& other) :
        name { other.name },
        type { other.type ? std::make_unique<Type>(*other.type) : nullptr },
        offsetBytes { other.offsetBytes }
{
}

Type::Member& Type::Member::operator=(const Member& other) {
    if (this != &other) {
        name = other.name;
        type = other.type ? std::make_unique<Type>(*other.type) : nullptr;
        offsetBytes = other.offsetBytes;
    }
    return *this;
}

namespace {
constexpr int MAX_ALIGN = 8;

int alignUp(int value, int alignment) {
    if (alignment < 1) {
        alignment = 1;
    }
    return (value + alignment - 1) / alignment * alignment;
}

// System V AMD64 natural alignment for struct/union members (matches libc ABI).
// Required so fstat/struct stat field offsets agree with glibc.
int typeAlignment(const Type& t) {
    if (t.isPointer()) {
        return 8;
    }
    if (t.isArray()) {
        return typeAlignment(t.getElementType());
    }
    if (t.isRecord()) {
        int align = 1;
        for (const auto& member : t.getStructMembers()) {
            if (!member.type) {
                continue;
            }
            int a = typeAlignment(*member.type);
            if (a > align) {
                align = a;
            }
        }
        if (align < 1) {
            align = 1;
        }
        if (align > MAX_ALIGN) {
            align = MAX_ALIGN;
        }
        return align;
    }
    if (t.isPrimitive()) {
        int size = t.getSize();
        if (size >= 8) {
            return 8;
        }
        if (size >= 4) {
            return 4;
        }
        if (size >= 2) {
            return 2;
        }
        return 1;
    }
    return 8;
}

int memberSize(const Type& memberType) {
    int size = memberType.getSize();
    // Incomplete / zero-size (flexible array) contributes no storage.
    if (size < 0) {
        return 0;
    }
    return size;
}

void layoutMembers(Type::StructBody& body, std::vector<std::pair<std::string, Type>> members) {
    body.members.clear();
    body.isUnion = false;
    int offset = 0;
    int maxAlign = 1;
    for (auto& entry : members) {
        int align = typeAlignment(entry.second);
        if (align > maxAlign) {
            maxAlign = align;
        }
        offset = alignUp(offset, align);
        body.members.emplace_back(entry.first, entry.second, offset);
        offset += memberSize(entry.second);
    }
    body.size = alignUp(offset, maxAlign);
    body.complete = true;
}

void layoutUnionMembers(Type::StructBody& body, std::vector<std::pair<std::string, Type>> members) {
    body.members.clear();
    body.isUnion = true;
    int maxSize = 0;
    int maxAlign = 1;
    for (auto& entry : members) {
        body.members.emplace_back(entry.first, entry.second, 0);
        int size = memberSize(entry.second);
        int align = typeAlignment(entry.second);
        if (size > maxSize) {
            maxSize = size;
        }
        if (align > maxAlign) {
            maxAlign = align;
        }
    }
    body.size = alignUp(maxSize, maxAlign);
    body.complete = true;
}
} // namespace

Type incompleteStructure() {
    Type result { std::vector<Qualifier> {} };
    result._structure = std::make_shared<Type::StructBody>();
    result._structure->complete = false;
    result._structure->size = 0;
    result._size = 0;
    return result;
}

Type structure(std::vector<std::pair<std::string, Type>> members) {
    Type result = incompleteStructure();
    completeStructure(result, std::move(members));
    return result;
}

void completeStructure(Type& structType, std::vector<std::pair<std::string, Type>> members) {
    if (!structType._structure) {
        structType._structure = std::make_shared<Type::StructBody>();
    }
    layoutMembers(*structType._structure, std::move(members));
    structType._size = structType._structure->size;
}

Type unionType(std::vector<std::pair<std::string, Type>> members) {
    Type result = incompleteStructure();
    completeUnion(result, std::move(members));
    return result;
}

void completeUnion(Type& unionTy, std::vector<std::pair<std::string, Type>> members) {
    if (!unionTy._structure) {
        unionTy._structure = std::make_shared<Type::StructBody>();
    }
    layoutUnionMembers(*unionTy._structure, std::move(members));
    unionTy._size = unionTy._structure->size;
}

bool Type::isRecord() const {
    // Pointer-to-struct/union keeps _structure for dereference(); it is not a record type.
    // Same rule as isArray/isFunction (git parse_event_data brace-init clobber).
    return _structure != nullptr && _indirection == 0;
}

bool Type::isStructure() const {
    return isRecord() && !_structure->isUnion;
}

bool Type::isUnion() const {
    return isRecord() && _structure->isUnion;
}

bool Type::isAggregate() const {
    return isArray() || isRecord();
}

bool Type::isCompleteRecord() const {
    return isRecord() && _structure->complete;
}

const void* Type::structureBodyIdentity() const {
    return _structure.get();
}

const std::vector<Type::Member>& Type::getStructMembers() const {
    return _structure->members;
}

bool Type::memberOffset(const std::string& memberName, int& offsetBytes) const {
    if (!_structure) {
        return false;
    }
    for (const auto& member : _structure->members) {
        if (!member.name.empty() && member.name == memberName) {
            offsetBytes = member.offsetBytes;
            return true;
        }
        // C11 anonymous struct/union: fields are visible in the enclosing type.
        if (member.name.empty() && member.type && member.type->isRecord()) {
            int nestedOff = 0;
            if (member.type->memberOffset(memberName, nestedOff)) {
                offsetBytes = member.offsetBytes + nestedOff;
                return true;
            }
        }
    }
    return false;
}

bool Type::memberType(const std::string& memberName, Type& outType) const {
    if (!_structure) {
        return false;
    }
    for (const auto& member : _structure->members) {
        if (!member.name.empty() && member.name == memberName) {
            outType = *member.type;
            return true;
        }
        if (member.name.empty() && member.type && member.type->isRecord()) {
            if (member.type->memberType(memberName, outType)) {
                return true;
            }
        }
    }
    return false;
}

Type builtinVaListTagType() {
    // Layout matches glibc / System V AMD64 ABI (24 bytes).
    return structure({
            { "gp_offset", unsignedInteger() },
            { "fp_offset", unsignedInteger() },
            { "overflow_arg_area", pointer(voidType()) },
            { "reg_save_area", pointer(voidType()) },
    });
}

Type builtinVaListType() {
    return array(builtinVaListTagType(), 1);
}

} // namespace type
