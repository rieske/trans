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

int typeAlignment(const Type& t) {
    if (t.isPointer()) {
        return POINTER_SIZE;
    }
    if (t.isArray()) {
        return typeAlignment(t.getElementType());
    }
    if (t.isRecord()) {
        if (t.isPacked()) {
            return 1;
        }
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
        int align = t.getPrimitive().getAlignment();
        return align < 1 ? 1 : align;
    }
    return 1;
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
    // After incomplete rejection, use raw element size (may be 0 for empty complete records).
    const long long stride = elementType.getSize();
    const long long bytes = stride * static_cast<long long>(elementCount);
    if (bytes > static_cast<long long>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument { "array size is too large" };
    }
    Type result { std::vector<Qualifier> {} };
    Type::ArrayPayload arr;
    arr.element = std::make_shared<Type>(elementType);
    arr.count = elementCount;
    arr.sizeBytes = static_cast<int>(bytes);
    arr.complete = true;
    result._payload = std::move(arr);
    return result;
}

Type incompleteArray(const Type& elementType) {
    if (isIncompleteMemberOrElementType(elementType)) {
        throw std::invalid_argument { "array of incomplete type" };
    }
    Type result { std::vector<Qualifier> {} };
    Type::ArrayPayload arr;
    arr.element = std::make_shared<Type>(elementType);
    arr.count = 0;
    arr.sizeBytes = 0;
    arr.complete = false;
    result._payload = std::move(arr);
    return result;
}

Type variableArray(const Type& elementType, std::shared_ptr<ast::Expression> bound) {
    if (isIncompleteMemberOrElementType(elementType)) {
        throw std::invalid_argument { "array of incomplete type" };
    }
    Type result { std::vector<Qualifier> {} };
    Type::ArrayPayload arr;
    arr.element = std::make_shared<Type>(elementType);
    arr.count = 0;
    arr.sizeBytes = 0;
    arr.complete = true;
    arr.variable = true;
    arr.bound = std::move(bound);
    result._payload = std::move(arr);
    return result;
}

Type signedCharacter(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedCharacter(), qualifiers);
}
Type unsignedCharacter(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedCharacter(), qualifiers);
}
Type boolean(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::boolean(), qualifiers);
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
Type signedInt128(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::signedInt128(), qualifiers);
}
Type unsignedInt128(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::unsignedInt128(), qualifiers);
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
Type complexFloat(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::complexFloat(), qualifiers);
}
Type complexDouble(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::complexDouble(), qualifiers);
}
Type complexLongDouble(const std::vector<Qualifier>& qualifiers) {
    return primitive(Primitive::complexLongDouble(), qualifiers);
}

void Type::applyQualifiers(const std::vector<Qualifier>& qualifiers) {
    for (const auto& qualifier : qualifiers) {
        switch (qualifier) {
            case Qualifier::CONST:
                _const = true;
                break;
            case Qualifier::VOLATILE:
                _volatile = true;
                break;
            case Qualifier::RESTRICT:
                break;
            default:
                throw std::invalid_argument { "Unsupported type qualifier" };
        }
    }
}

Type::Type(std::vector<Qualifier> qualifiers) : _payload { VoidPayload {} } {
    applyQualifiers(qualifiers);
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


namespace {

enum class ArrayBound {
    Exact,
    Compatible
};

bool sameShape(const Type& a, const Type& b, bool matchQualifiers, ArrayBound bounds) {
    if (matchQualifiers) {
        if (a.isConst() != b.isConst() || a.isVolatile() != b.isVolatile()) {
            return false;
        }
    }
    const Type left = matchQualifiers ? a : a.withoutTopLevelQualifiers();
    const Type right = matchQualifiers ? b : b.withoutTopLevelQualifiers();
    if (left.kind() != right.kind()) {
        return false;
    }
    switch (left.kind()) {
    case TypeKind::Void:
        return true;
    case TypeKind::Primitive:
        return left.getPrimitive().equivalentTo(right.getPrimitive());
    case TypeKind::Pointer:
        return sameShape(left.dereference(), right.dereference(), matchQualifiers, bounds);
    case TypeKind::Array:
        if (!sameShape(left.getElementType(), right.getElementType(), matchQualifiers, bounds)) {
            return false;
        }
        if (bounds == ArrayBound::Exact) {
            return left.isIncompleteArray() == right.isIncompleteArray()
                    && left.isVariableArray() == right.isVariableArray()
                    && (left.isVariableArray() || left.getArraySize() == right.getArraySize());
        }
        if (left.isVariableArray() || right.isVariableArray()) {
            return true;
        }
        if (!left.isIncompleteArray() && !right.isIncompleteArray()) {
            return left.getArraySize() == right.getArraySize();
        }
        return true;
    case TypeKind::Function: {
        const Function fa = left.getFunction();
        const Function fb = right.getFunction();
        if (fa.isVariadic() != fb.isVariadic()) {
            return false;
        }
        if (!sameShape(fa.getReturnType(), fb.getReturnType(), matchQualifiers, bounds)) {
            return false;
        }
        const auto aa = fa.getArguments();
        const auto ba = fb.getArguments();
        if (aa.size() != ba.size()) {
            return false;
        }
        for (std::size_t i = 0; i < aa.size(); ++i) {
            if (!sameShape(aa[i], ba[i], matchQualifiers, bounds)) {
                return false;
            }
        }
        return true;
    }
    case TypeKind::Struct:
    case TypeKind::Union:
        return left.structureBodyIdentity() == right.structureBodyIdentity();
    }
    return false;
}

std::vector<Qualifier> topQualifiers(const Type& t) {
    std::vector<Qualifier> quals;
    if (t.isConst()) {
        quals.push_back(Qualifier::CONST);
    }
    if (t.isVolatile()) {
        quals.push_back(Qualifier::VOLATILE);
    }
    return quals;
}

Type makeComposite(const Type& a, const Type& b) {
    switch (a.kind()) {
    case TypeKind::Array: {
        const Type element = makeComposite(a.getElementType(), b.getElementType());
        Type result = incompleteArray(element);
        if (a.isVariableArray() && b.isVariableArray()) {
            auto bound = a.variableBound();
            if (!bound) {
                bound = b.variableBound();
            }
            result = variableArray(element, std::move(bound));
        } else if (a.isVariableArray()) {
            result = b.isIncompleteArray() ? variableArray(element)
                    : array(element, b.getArraySize());
        } else if (b.isVariableArray()) {
            result = a.isIncompleteArray() ? variableArray(element)
                    : array(element, a.getArraySize());
        } else if (!a.isIncompleteArray()) {
            result = array(element, a.getArraySize());
        } else if (!b.isIncompleteArray()) {
            result = array(element, b.getArraySize());
        }
        return result.withQualifiers(topQualifiers(a));
    }
    case TypeKind::Pointer:
        return pointer(makeComposite(a.dereference(), b.dereference()), topQualifiers(a));
    case TypeKind::Function: {
        const Function fa = a.getFunction();
        const Function fb = b.getFunction();
        std::vector<Type> args;
        const auto aa = fa.getArguments();
        const auto ba = fb.getArguments();
        args.reserve(aa.size());
        for (std::size_t i = 0; i < aa.size(); ++i) {
            args.push_back(makeComposite(aa[i], ba[i]));
        }
        return function(makeComposite(fa.getReturnType(), fb.getReturnType()), args, fa.isVariadic())
                .withQualifiers(topQualifiers(a));
    }
    default:
        return a;
    }
}

} // namespace

bool Type::equivalentTo(const Type& other) const {
    return sameShape(*this, other, false, ArrayBound::Exact);
}

bool Type::sameQualifiedType(const Type& other) const {
    return sameShape(*this, other, true, ArrayBound::Exact);
}

bool Type::compatibleWith(const Type& other) const {
    return sameShape(*this, other, true, ArrayBound::Compatible);
}

std::optional<Type> Type::composite(const Type& other) const {
    if (!compatibleWith(other)) {
        return std::nullopt;
    }
    if (sameQualifiedType(other)) {
        return *this;
    }
    return makeComposite(*this, other);
}

bool Type::sameUnqualifiedType(const Type& other) const {
    return withoutTopLevelQualifiers().sameQualifiedType(other.withoutTopLevelQualifiers());
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

Type Type::withQualifiers(const std::vector<Qualifier>& qualifiers) const {
    Type t { *this };
    t.applyQualifiers(qualifiers);
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

bool Type::isIncompleteArray() const {
    const auto* a = arrayPayload();
    return a && !a->complete;
}

bool Type::isVariableArray() const {
    const auto* a = arrayPayload();
    return a && a->variable;
}

std::shared_ptr<ast::Expression> Type::variableBound() const {
    const auto* a = arrayPayload();
    if (a && a->variable) {
        return a->bound;
    }
    return {};
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
        return a->element->getSize();
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

std::optional<Type> Type::indexElement() const {
    if (isPointer()) {
        return dereference();
    }
    if (isArray()) {
        return getElementType();
    }
    return std::nullopt;
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
            if (t.isIncompleteArray()) {
                dims += "[]";
            } else if (t.isVariableArray()) {
                dims += "[*]";
            } else {
                dims += "[" + std::to_string(t.getArraySize()) + "]";
            }
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

Type::Member::Member(std::string n, Type t, int off, std::optional<BitField> bits) :
        name { std::move(n) },
        type { std::make_unique<Type>(std::move(t)) },
        offsetBytes { off },
        bitField { std::move(bits) }
{
}

Type::Member::Member(const Member& other) :
        name { other.name },
        type { other.type ? std::make_unique<Type>(*other.type) : nullptr },
        offsetBytes { other.offsetBytes },
        bitField { other.bitField }
{
}

Type::Member& Type::Member::operator=(const Member& other) {
    if (this != &other) {
        name = other.name;
        type = other.type ? std::make_unique<Type>(*other.type) : nullptr;
        offsetBytes = other.offsetBytes;
        bitField = other.bitField;
    }
    return *this;
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

Type incompleteRecord() {
    Type result { std::vector<Qualifier> {} };
    Type::RecordPayload rec;
    rec.body = std::make_shared<Type::StructBody>();
    rec.body->complete = false;
    rec.body->size = 0;
    result._payload = std::move(rec);
    return result;
}

namespace {

std::vector<MemberSpec> specsFromPairs(const std::vector<std::pair<std::string, Type>>& members) {
    std::vector<MemberSpec> specs;
    specs.reserve(members.size());
    for (const auto& [name, memberType] : members) {
        specs.push_back(MemberSpec { name, memberType });
    }
    return specs;
}

} // namespace

Type structure(const std::vector<std::pair<std::string, Type>>& members) {
    Type result = incompleteRecord();
    completeStructure(result, specsFromPairs(members));
    return result;
}

Type unionType(const std::vector<std::pair<std::string, Type>>& members) {
    Type result = incompleteRecord();
    completeUnion(result, specsFromPairs(members));
    return result;
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

bool Type::isTransparentUnion() const {
    const auto* b = body();
    return b && b->isUnion && b->transparentUnion;
}

void Type::markTransparentUnion() {
    auto* b = body();
    if (b && b->isUnion) {
        b->transparentUnion = true;
    }
}

bool Type::isPacked() const {
    const auto* b = body();
    return b && b->packed;
}

void Type::applyPacked() {
    auto* b = body();
    if (!b || b->packed) {
        return;
    }
    if (!b->complete) {
        b->packed = true;
        return;
    }
    std::vector<MemberSpec> specs;
    specs.reserve(b->members.size());
    for (const auto& member : b->members) {
        std::optional<int> width;
        if (member.bitField) {
            width = member.bitField->width;
        }
        specs.emplace_back(member.name, *member.type, width);
    }
    completeRecord(*this, specs, true);
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

std::optional<MemberPath> lookupMemberPath(const Type& record, const std::string& name) {
    const int n = record.memberCount();
    for (int i = 0; i < n; ++i) {
        auto member = memberAt(record, i);
        if (!member) {
            continue;
        }
        if (!member->name.empty() && member->name == name) {
            return MemberPath { std::move(*member), { i } };
        }
        if (member->name.empty() && member->type.isRecord()) {
            if (auto nested = lookupMemberPath(member->type, name)) {
                nested->member.offsetBytes += member->offsetBytes;
                nested->indices.insert(nested->indices.begin(), i);
                return nested;
            }
        }
    }
    return std::nullopt;
}

std::optional<FoundMember> lookupMember(const Type& record, const std::string& name) {
    if (auto path = lookupMemberPath(record, name)) {
        return std::move(path->member);
    }
    return std::nullopt;
}

std::optional<FoundMember> memberAt(const Type& record, int index) {
    if (!record.isRecord() || index < 0 || index >= record.memberCount()) {
        return std::nullopt;
    }
    const auto& member = record.getMembers()[static_cast<std::size_t>(index)];
    return FoundMember {
            member.name,
            member.type ? *member.type : voidType(),
            member.offsetBytes,
            member.bitField };
}

BitField makeBitField(const Type& declared, int width, int shift) {
    BitField bits;
    bits.width = width;
    bits.shift = shift;
    bits.isSigned = valueIsSigned(declared);
    return bits;
}

OffsetofResult resolveOffsetof(const Type& record, const std::string& name) {
    if (!record.isCompleteRecord()) {
        return { OffsetofStatus::Incomplete, 0 };
    }
    const auto found = lookupMember(record, name);
    if (!found) {
        return { OffsetofStatus::Missing, 0 };
    }
    if (found->isBitField()) {
        return { OffsetofStatus::BitField, 0 };
    }
    return { OffsetofStatus::Ok, found->offsetBytes };
}

int Type::memberCount() const {
    if (!isRecord()) {
        return 0;
    }
    return static_cast<int>(getMembers().size());
}

} // namespace type
