#include "TypeInfo.h"

namespace semantic_analyzer {

TypeInfo::TypeInfo(BasicType basicType, std::string extendedType) :
        basicType { basicType },
        extendedType { extendedType } {
}

BasicType TypeInfo::getBasicType() const {
    return basicType;
}

std::string TypeInfo::getExtendedType() const {
    return extendedType;
}

TypeInfo::~TypeInfo() {
}

} /* namespace semantic_analyzer */
