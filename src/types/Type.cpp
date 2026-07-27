#include "Type.h"
#include "TypeQuery.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace type {

static const int POINTER_SIZE { 8 };

namespace {

// Storage size of one array element in bytes (at least 1 for empty-ish types).
int elementStrideBytes(const Type& elementType) {
    int size = elementType.getSize();
    return size < 1 ? 1 : size;
}

long long alignUp(long long offset, int alignment) {
    if (alignment <= 1) {
        return offset;
    }
    const long long rem = offset % alignment;
    return rem == 0 ? offset : offset + (alignment - rem);
}

int typeAlignment(const Type& t) {
    if (t.isPointer()) {
        return POINTER_SIZE;
    }
    if (t.isArray()) {
        return typeAlignment(t.getElementType());
    }
    if (t.isRecord()) {
        int align = 1;
        for (const auto& member : t.getMembers()) {
            if (!member.type) {
                continue;
            }
            int a = typeAlignment(*member.type);
            if (a > align) {
                align = a;
            }
        }
        return align < 1 ? 1 : align;
    }
    if (t.isPrimitive()) {
        // Natural alignment equals size for the primitives we model (1/2/4/8/16).
        int size = t.getSize();
        if (size >= 1) {
            return size;
        }
        return 1;
    }
    return 1;
}

int memberSize(const Type& memberType) {
    int size = memberType.getSize();
    return size < 0 ? 0 : size;
}

bool isIncompleteMemberType(const Type& memberType) {
    return isIncompleteMemberOrElementType(memberType);
}

void validateAndLayoutMembers(Type::StructBody& body,
        const std::vector<std::pair<std::string, Type>>& members,
        bool asUnion) {
    body.members.clear();
    body.isUnion = asUnion;
    long long offset = 0;
    int maxAlign = 1;
    long long maxSize = 0;

    for (const auto& [name, memberType] : members) {
        if (isIncompleteMemberType(memberType)) {
            throw std::invalid_argument { asUnion
                    ? "union member has incomplete type"
                    : "structure member has incomplete type" };
        }
        for (const auto& existing : body.members) {
            if (!name.empty() && existing.name == name) {
                throw std::invalid_argument { asUnion
                        ? "duplicate union member name"
                        : "duplicate structure member name" };
            }
        }
        const int align = typeAlignment(memberType);
        if (align > maxAlign) {
            maxAlign = align;
        }
        if (asUnion) {
            body.members.emplace_back(name, memberType, 0);
            long long size = memberSize(memberType);
            if (size > maxSize) {
                maxSize = size;
            }
        } else {
            offset = alignUp(offset, align);
            if (offset > static_cast<long long>(std::numeric_limits<int>::max())) {
                throw std::invalid_argument { "structure size is too large" };
            }
            body.members.emplace_back(name, memberType, static_cast<int>(offset));
            offset += memberSize(memberType);
            if (offset > static_cast<long long>(std::numeric_limits<int>::max())) {
                throw std::invalid_argument { "structure size is too large" };
            }
        }
    }

    if (asUnion) {
        long long size = alignUp(maxSize, maxAlign);
        if (size > static_cast<long long>(std::numeric_limits<int>::max())) {
            throw std::invalid_argument { "union size is too large" };
        }
        body.size = static_cast<int>(size);
    } else {
        offset = alignUp(offset, maxAlign);
        if (offset > static_cast<long long>(std::numeric_limits<int>::max())) {
            throw std::invalid_argument { "structure size is too large" };
        }
        body.size = static_cast<int>(offset);
    }
    body.complete = true;
}

} // namespace

Type voidType() {
    return Type { std::vector<Qualifier> {} };
}

Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers) {
    return Type { primitive, qualifiers };
}

Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers) {
    Type p { qualifiers };
    p._payload = Type::PointerPayload { std::make_shared<Type>(pointsTo) };
    return p;
}

Type function(const Type& returnType, const std::vector<Type>& arguments, bool variadic) {
    return Type { returnType, arguments, variadic };
}

Type array(const Type& elementType, int elementCount) {
    if (elementCount < 0) {
        throw std::invalid_argument { "array size must be non-negative" };
    }
    // Complete object element: void, bare function, and incomplete records are rejected.
    if (isIncompleteMemberOrElementType(elementType)) {
        throw std::invalid_argument { "array of incomplete type" };
    }
    const long long stride = elementStrideBytes(elementType);
    const long long bytes = stride * static_cast<long long>(elementCount);
    if (bytes > static_cast<long long>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument { "array size is too large" };
    }
    Type result { std::vector<Qualifier> {} };
    Type::ArrayPayload arr;
    arr.element = std::make_shared<Type>(elementType);
    arr.count = elementCount;
    arr.sizeBytes = static_cast<int>(bytes);
    result._payload = std::move(arr);
    return result;
}

Type signedCharacter(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedCharacter(), qualifiers);
}
Type unsignedCharacter(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedCharacter(), qualifiers);
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

Type::Type(std::vector<Qualifier> qualifiers) : _payload { VoidPayload {} } {
    for (const auto& qualifier : qualifiers) {
        switch (qualifier) {
            case Qualifier::CONST:
                this->_const = true;
                break;
            case Qualifier::VOLATILE:
                this->_volatile = true;
                break;
            default:
                throw std::invalid_argument { "Unsupported type qualifier" };
        }
    }
}

Type::Type(const Primitive& primitive, std::vector<Qualifier> qualifiers) :
        Type { qualifiers }
{
    _payload = PrimitivePayload { primitive };
}

Type::Type(const Type& returnType, const std::vector<Type>& arguments, bool variadic) {
    std::vector<std::unique_ptr<Type>> args;
    for (const auto& arg : arguments) {
        args.push_back(std::make_unique<Type>(arg));
    }
    _payload = FunctionPayload {
            Function { std::make_unique<Type>(returnType), std::move(args), variadic } };
}

const Type::RecordPayload* Type::recordPayload() const {
    return std::get_if<RecordPayload>(&_payload);
}

Type::RecordPayload* Type::recordPayload() {
    return std::get_if<RecordPayload>(&_payload);
}

const Type::StructBody* Type::body() const {
    const auto* rec = recordPayload();
    return rec ? rec->body.get() : nullptr;
}

Type::StructBody* Type::body() {
    auto* rec = recordPayload();
    return rec ? rec->body.get() : nullptr;
}

const Type::ArrayPayload* Type::arrayPayload() const {
    return std::get_if<ArrayPayload>(&_payload);
}

int Type::getSize() const {
    if (std::holds_alternative<PointerPayload>(_payload)) {
        return POINTER_SIZE;
    }
    if (const auto* a = arrayPayload()) {
        return a->sizeBytes;
    }
    if (const auto* prim = std::get_if<PrimitivePayload>(&_payload)) {
        return prim->value.getSize();
    }
    if (const auto* b = body()) {
        return b->size;
    }
    return 0;
}

int Type::getAlignment() const {
    return typeAlignment(*this);
}

bool Type::canAssignFrom(const Type& other) const {
    return productCanAssignFrom(*this, other);
}

bool Type::equivalentTo(const Type& other) const {
    Type a = withoutTopLevelQualifiers();
    Type b = other.withoutTopLevelQualifiers();
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
        return a.getPrimitive().getSize() == b.getPrimitive().getSize()
                && a.getPrimitive().isSigned() == b.getPrimitive().isSigned()
                && a.getPrimitive().isFloating() == b.getPrimitive().isFloating();
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
        // Unreachable: pointer layers are peeled above.
        return false;
    }
    return false;
}

TypeKind Type::kind() const {
    return std::visit([](const auto& arm) -> TypeKind {
        using T = std::decay_t<decltype(arm)>;
        if constexpr (std::is_same_v<T, VoidPayload>) {
            return TypeKind::Void;
        } else if constexpr (std::is_same_v<T, PrimitivePayload>) {
            return TypeKind::Primitive;
        } else if constexpr (std::is_same_v<T, PointerPayload>) {
            return TypeKind::Pointer;
        } else if constexpr (std::is_same_v<T, FunctionPayload>) {
            return TypeKind::Function;
        } else if constexpr (std::is_same_v<T, ArrayPayload>) {
            return TypeKind::Array;
        } else if constexpr (std::is_same_v<T, RecordPayload>) {
            // Null body is still a record placeholder; treat as Struct until completed as union.
            return arm.body && arm.body->isUnion ? TypeKind::Union : TypeKind::Struct;
        }
    }, _payload);
}

bool Type::isVoid() const {
    return kind() == TypeKind::Void;
}

bool Type::isPrimitive() const {
    // Pointers are PointerPayload (not Primitive). Peeling uses dereference().
    return kind() == TypeKind::Primitive;
}

Primitive Type::getPrimitive() const {
    if (const auto* p = std::get_if<PrimitivePayload>(&_payload)) {
        return p->value;
    }
    throw std::domain_error { "getPrimitive on non-primitive type" };
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
    return std::holds_alternative<PointerPayload>(_payload);
}

bool Type::isFunction() const {
    return kind() == TypeKind::Function;
}

Function Type::getFunction() const {
    if (const auto* f = std::get_if<FunctionPayload>(&_payload)) {
        return f->value;
    }
    throw std::domain_error { "getFunction on non-function type" };
}

bool Type::isArray() const {
    return kind() == TypeKind::Array;
}

Type Type::getElementType() const {
    if (const auto* a = arrayPayload()) {
        return *a->element;
    }
    throw std::domain_error { "not an array type" };
}

int Type::getArraySize() const {
    if (const auto* a = arrayPayload()) {
        return a->count;
    }
    throw std::domain_error { "not an array type" };
}

Type Type::decayArray() const {
    if (!isArray()) {
        throw std::domain_error { "not an array type" };
    }
    return pointer(getElementType());
}

int Type::getElementStride() const {
    if (const auto* a = arrayPayload()) {
        return elementStrideBytes(*a->element);
    }
    throw std::domain_error { "not an array type" };
}

Type Type::dereference() const {
    if (const auto* p = std::get_if<PointerPayload>(&_payload)) {
        if (p->pointee) {
            return *p->pointee;
        }
    }
    throw std::domain_error { "can not dereference non-pointer type" };
}

std::string Type::to_string() const {
    if (isVoid()) {
        return "void";
    }
    if (isPointer()) {
        return dereference().to_string() + "*";
    }
    if (isArray()) {
        Type t = *this;
        std::string dims;
        while (t.isArray()) {
            dims += "[" + std::to_string(t.getArraySize()) + "]";
            t = t.getElementType();
        }
        return t.to_string() + dims;
    }
    if (isPrimitive()) {
        std::stringstream str;
        if (isConst()) {
            str << "const ";
        }
        if (isVolatile()) {
            str << "volatile ";
        }
        str << getPrimitive().to_string();
        return str.str();
    }
    if (isFunction()) {
        return getFunction().to_string();
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

Type incompleteRecord() {
    Type result { std::vector<Qualifier> {} };
    Type::RecordPayload rec;
    rec.body = std::make_shared<Type::StructBody>();
    rec.body->complete = false;
    rec.body->size = 0;
    result._payload = std::move(rec);
    return result;
}

Type structure(const std::vector<std::pair<std::string, Type>>& members) {
    Type result = incompleteRecord();
    completeStructure(result, members);
    return result;
}

void completeStructure(Type& structType,
        const std::vector<std::pair<std::string, Type>>& members) {
    // Friend of Type: require an existing record body; do not invent one on non-records.
    auto* rec = std::get_if<Type::RecordPayload>(&structType._payload);
    if (!rec || !rec->body) {
        throw std::domain_error { "completeStructure on non-record type" };
    }
    validateAndLayoutMembers(*rec->body, members, false);
}

Type unionType(const std::vector<std::pair<std::string, Type>>& members) {
    Type result = incompleteRecord();
    completeUnion(result, members);
    return result;
}

void completeUnion(Type& unionTy,
        const std::vector<std::pair<std::string, Type>>& members) {
    auto* rec = std::get_if<Type::RecordPayload>(&unionTy._payload);
    if (!rec || !rec->body) {
        throw std::domain_error { "completeUnion on non-record type" };
    }
    validateAndLayoutMembers(*rec->body, members, true);
}

bool Type::isRecord() const {
    return recordPayload() != nullptr;
}

bool Type::isStructure() const {
    const auto* b = body();
    return b && !b->isUnion;
}

bool Type::isUnion() const {
    const auto* b = body();
    return b && b->isUnion;
}

bool Type::isAggregate() const {
    return isArray() || isRecord();
}

bool Type::isCompleteRecord() const {
    const auto* b = body();
    return b && b->complete;
}

bool Type::isIncompleteRecord() const {
    return isRecord() && !isCompleteRecord();
}

void Type::completeStructure(const std::vector<std::pair<std::string, Type>>& members) {
    type::completeStructure(*this, members);
}

const void* Type::structureBodyIdentity() const {
    const auto* b = body();
    return b;
}

const std::vector<Type::Member>& Type::getMembers() const {
    const auto* b = body();
    if (!b) {
        static const std::vector<Member> empty;
        return empty;
    }
    return b->members;
}

bool Type::memberOffset(const std::string& memberName, int& offsetBytes) const {
    const auto* b = body();
    if (!b) {
        return false;
    }
    for (const auto& member : b->members) {
        if (!member.name.empty() && member.name == memberName) {
            offsetBytes = member.offsetBytes;
            return true;
        }
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
    const auto* b = body();
    if (!b) {
        return false;
    }
    for (const auto& member : b->members) {
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

int Type::memberCount() const {
    if (!isRecord()) {
        return 0;
    }
    return static_cast<int>(getMembers().size());
}

bool Type::memberAt(int index, std::string& name, Type& outType, int& offsetBytes) const {
    if (!isRecord() || index < 0 || index >= memberCount()) {
        return false;
    }
    const auto& m = getMembers()[static_cast<std::size_t>(index)];
    name = m.name;
    outType = m.type ? *m.type : voidType();
    offsetBytes = m.offsetBytes;
    return true;
}

} // namespace type
