#include "Type.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace type {

static const int POINTER_SIZE {8};

namespace {
int alignedStride(const Type& elementType) {
    int size = elementType.getSize();
    if (size < 1) {
        size = 1;
    }
    return size;
}

// Local aliases for payload arms (Type:: private nested types are accessible in members only).
// Free functions use public Type API where possible.
} // namespace

Type voidType() {
    return Type{std::vector<Qualifier>{}};
}

Type primitive(const Primitive& primitive, const std::vector<Qualifier>& qualifiers) {
    return Type{primitive, qualifiers};
}

Type pointer(const Type& pointsTo, const std::vector<Qualifier>& qualifiers) {
    Type p { std::vector<Qualifier> {} };
    p._payload = Type::PointerPayload{ std::make_shared<Type>(pointsTo) };
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

Type array(const Type& elementType, int elementCount) {
    if (elementCount < 0) {
        throw std::invalid_argument { "array size must be non-negative" };
    }
    // Checked size: huge constants (e.g. int v[INT_MAX]) must not UBSAN-overflow
    // stride * count (mutfuzz frontend bucket).
    const long long stride = alignedStride(elementType);
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

Type::Type(std::vector<Qualifier> qualifiers) : _payload { VoidPayload{} } {
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
    _payload = PrimitivePayload{ primitive };
}

Type::Type(const Type& returnType, const std::vector<Type>& arguments, bool variadic) {
    std::vector<std::unique_ptr<Type>> args;
    for (const auto& arg : arguments) {
        args.push_back(std::make_unique<Type>(arg));
    }
    _payload = FunctionPayload{
            Function{std::make_unique<Type>(returnType), std::move(args), variadic}};
}

int Type::getSize() const {
    if (const auto* p = std::get_if<PointerPayload>(&_payload)) {
        (void)p;
        return POINTER_SIZE;
    }
    if (const auto* a = std::get_if<ArrayPayload>(&_payload)) {
        return a->sizeBytes;
    }
    if (const auto* prim = std::get_if<PrimitivePayload>(&_payload)) {
        return prim->value.getSize();
    }
    if (const auto* rec = std::get_if<RecordPayload>(&_payload)) {
        return rec->body ? rec->body->size : 0;
    }
    return 0;
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
            return arm.body && arm.body->isUnion ? TypeKind::Union : TypeKind::Struct;
        }
    }, _payload);
}

bool Type::isVoid() const {
    return kind() == TypeKind::Void;
}

bool Type::isPrimitive() const {
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
    if (const auto* a = std::get_if<ArrayPayload>(&_payload)) {
        return *a->element;
    }
    throw std::domain_error { "not an array type" };
}

int Type::getArraySize() const {
    if (const auto* a = std::get_if<ArrayPayload>(&_payload)) {
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
    if (const auto* a = std::get_if<ArrayPayload>(&_payload)) {
        return alignedStride(*a->element);
    }
    throw std::domain_error { "not an array type" };
}

Type Type::dereference() const {
    if (const auto* p = std::get_if<PointerPayload>(&_payload)) {
        if (p->pointee) {
            return *p->pointee;
        }
    }
    throw std::domain_error{"can not dereference non-pointer type"};
}

std::string Type::to_string() const {
    if (isVoid()) {
        return "void";
    }
    if (isPointer()) {
        return dereference().to_string() + "*";
    }
    if (isArray()) {
        return getElementType().to_string() + "[" + std::to_string(getArraySize()) + "]";
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

namespace {
constexpr int MAX_ALIGN = 8;

int alignUp(int value, int alignment) {
    if (alignment < 1) {
        alignment = 1;
    }
    return (value + alignment - 1) / alignment * alignment;
}

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
    Type::RecordPayload rec;
    rec.body = std::make_shared<Type::StructBody>();
    rec.body->complete = false;
    rec.body->size = 0;
    result._payload = std::move(rec);
    return result;
}

Type structure(std::vector<std::pair<std::string, Type>> members) {
    Type result = incompleteStructure();
    completeStructure(result, std::move(members));
    return result;
}

void completeStructure(Type& structType, std::vector<std::pair<std::string, Type>> members) {
    auto* rec = std::get_if<Type::RecordPayload>(&structType._payload);
    if (!rec || !rec->body) {
        Type::RecordPayload fresh;
        fresh.body = std::make_shared<Type::StructBody>();
        structType._payload = std::move(fresh);
        rec = std::get_if<Type::RecordPayload>(&structType._payload);
    }
    layoutMembers(*rec->body, std::move(members));
}

Type unionType(std::vector<std::pair<std::string, Type>> members) {
    Type result = incompleteStructure();
    completeUnion(result, std::move(members));
    return result;
}

void completeUnion(Type& unionTy, std::vector<std::pair<std::string, Type>> members) {
    auto* rec = std::get_if<Type::RecordPayload>(&unionTy._payload);
    if (!rec || !rec->body) {
        Type::RecordPayload fresh;
        fresh.body = std::make_shared<Type::StructBody>();
        unionTy._payload = std::move(fresh);
        rec = std::get_if<Type::RecordPayload>(&unionTy._payload);
    }
    layoutUnionMembers(*rec->body, std::move(members));
}

bool Type::isRecord() const {
    return kind() == TypeKind::Struct || kind() == TypeKind::Union;
}

bool Type::isStructure() const {
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    return rec && rec->body && !rec->body->isUnion;
}

bool Type::isUnion() const {
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    return rec && rec->body && rec->body->isUnion;
}

bool Type::isAggregate() const {
    return isArray() || isRecord();
}

bool Type::isCompleteRecord() const {
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    return rec && rec->body && rec->body->complete;
}

const void* Type::structureBodyIdentity() const {
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    return rec && rec->body ? rec->body.get() : nullptr;
}

const std::vector<Type::Member>& Type::getStructMembers() const {
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    if (!rec || !rec->body) {
        static const std::vector<Member> empty;
        return empty;
    }
    return rec->body->members;
}

bool Type::memberOffset(const std::string& memberName, int& offsetBytes) const {
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    if (!rec || !rec->body) {
        return false;
    }
    for (const auto& member : rec->body->members) {
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
    const auto* rec = std::get_if<RecordPayload>(&_payload);
    if (!rec || !rec->body) {
        return false;
    }
    for (const auto& member : rec->body->members) {
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
