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
    p._indirection = pointsTo._indirection+1;
    return p;
}

Type function(const Type& returnType, const std::vector<Type>& arguments) {
    return Type{returnType, arguments};
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

Type::Type(const Type& returnType, const std::vector<Type>& arguments) {
    std::vector<std::unique_ptr<Type>> args;
    for (const auto& arg : arguments) {
        args.push_back(std::make_unique<Type>(arg));
    }
    _function = Function{std::make_unique<Type>(returnType), std::move(args)};
}

int Type::getSize() const {
    if (isPointer()) {
        return POINTER_SIZE;
    }
    if (isPrimitive()) {
        return _primitive->getSize();
    }
    if (isStructure()) {
        return _structure->size;
    }
    return _size;
}

bool Type::canAssignFrom(const Type& other) const {
    // TODO:
    return true;
}

bool Type::isVoid() const {
    return !isPrimitive() && !isFunction() && !isPointer() && !isStructure();
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

bool Type::isPointer() const {
    return _indirection;
}

bool Type::isFunction() const {
    return _function.has_value();
}

Function Type::getFunction() const {
    return *_function;
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
constexpr int WORD_ALIGN = 8;

int alignUp(int value, int alignment) {
    return (value + alignment - 1) / alignment * alignment;
}

int memberStride(const Type& memberType) {
    int size = memberType.getSize();
    if (size < 1) {
        size = 1;
    }
    // Pointers and incomplete structs still get a full word as a member slot.
    return alignUp(size, WORD_ALIGN);
}

void layoutMembers(Type::StructBody& body, std::vector<std::pair<std::string, Type>> members) {
    body.members.clear();
    int offset = 0;
    for (auto& entry : members) {
        offset = alignUp(offset, WORD_ALIGN);
        body.members.emplace_back(entry.first, entry.second, offset);
        offset += memberStride(entry.second);
    }
    body.size = alignUp(offset, WORD_ALIGN);
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

bool Type::isStructure() const {
    return _structure != nullptr;
}

bool Type::isCompleteStructure() const {
    return _structure && _structure->complete;
}

const std::vector<Type::Member>& Type::getStructMembers() const {
    return _structure->members;
}

bool Type::memberOffset(const std::string& memberName, int& offsetBytes) const {
    if (!_structure) {
        return false;
    }
    for (const auto& member : _structure->members) {
        if (member.name == memberName) {
            offsetBytes = member.offsetBytes;
            return true;
        }
    }
    return false;
}

bool Type::memberType(const std::string& memberName, Type& outType) const {
    if (!_structure) {
        return false;
    }
    for (const auto& member : _structure->members) {
        if (member.name == memberName) {
            outType = *member.type;
            return true;
        }
    }
    return false;
}

} // namespace type
