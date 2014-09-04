#include "TypeInfo.h"

namespace ast {

TypeInfo::TypeInfo(BasicType basicType, std::string extendedType) :
        basicType { basicType },
        extendedType { extendedType } {
}

bool TypeInfo::isPlainVoid() const {
    return basicType == BasicType::VOID && extendedType.empty();
}

bool TypeInfo::isPlainInteger() const {
    return basicType == BasicType::INTEGER && extendedType.empty();
}

bool TypeInfo::isPointer() const {
    return extendedType.size() && (extendedType.at(0) == 'p' || extendedType.at(0) == 'a');
}

TypeInfo TypeInfo::dereference() const {
    return {basicType, extendedType.substr(1, extendedType.size())};
}

TypeInfo TypeInfo::point() const {
    return {basicType, "p" + extendedType};
}

bool TypeInfo::isVoidPointer() const {
    return basicType == BasicType::VOID && isPointer();
}

BasicType TypeInfo::getBasicType() const {
    return basicType;
}

std::string TypeInfo::getExtendedType() const {
    return extendedType;
}

TypeInfo::~TypeInfo() {
}

} /* namespace ast */
