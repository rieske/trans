#include "TypeInfo.h"

namespace ast {

TypeInfo::TypeInfo(BasicType basicType, int dereferenceCount) :
        basicType { basicType }, dereferenceCount { dereferenceCount } {
}

bool TypeInfo::isPlainVoid() const {
    return basicType == BasicType::VOID && !isPointer();
}

bool TypeInfo::isPlainInteger() const {
    return basicType == BasicType::INTEGER && !isPointer();
}

bool TypeInfo::isPointer() const {
    return dereferenceCount;
}

TypeInfo TypeInfo::dereference() const {
    return {basicType, dereferenceCount-1};
}

TypeInfo TypeInfo::point() const {
    return {basicType, dereferenceCount+1};
}

bool TypeInfo::isVoidPointer() const {
    return basicType == BasicType::VOID && isPointer();
}

BasicType TypeInfo::getBasicType() const {
    return basicType;
}

int TypeInfo::getDereferenceCount() const {
    return dereferenceCount;
}

bool TypeInfo::operator==(const TypeInfo& rhs) const {
    return basicType == rhs.basicType && dereferenceCount == rhs.dereferenceCount;
}

TypeInfo::~TypeInfo() {
}

} /* namespace ast */
