#include "Type.h"
#include "TypeQuery.h"

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
        throw std::logic_error { "array size must be non-negative" };
    }
    Type result { std::vector<Qualifier> {} };
    Type::ArrayPayload arr;
    arr.element = std::make_shared<Type>(elementType);
    arr.count = elementCount;
    arr.sizeBytes = arrayByteSize(elementType, elementCount).value_or(0);
    arr.complete = true;
    result._payload = std::move(arr);
    return result;
}

Type incompleteArray(const Type& elementType) {
    Type result { std::vector<Qualifier> {} };
    Type::ArrayPayload arr;
    arr.element = std::make_shared<Type>(elementType);
    arr.count = 0;
    arr.sizeBytes = 0;
    arr.complete = false;
    result._payload = std::move(arr);
    return result;
}

Type variableArray(const Type& elementType, std::shared_ptr<VlaBound> bound) {
    if (!bound) {
        bound = std::make_shared<VlaBound>();
        bound->unspecified = true;
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
                throw std::logic_error { "Unsupported type qualifier" };
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
    _payload = FunctionPayload { Function { returnType, arguments, variadic } };
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

const Function& Type::getFunction() const {
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

std::shared_ptr<VlaBound> Type::vlaBound() const {
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

} // namespace type
