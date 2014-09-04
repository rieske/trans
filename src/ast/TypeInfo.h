#ifndef TYPEINFO_H_
#define TYPEINFO_H_

#include <string>

#include "BasicType.h"

namespace ast {

class TypeInfo {
public:
    TypeInfo(BasicType basicType, std::string extendedType = "");
    virtual ~TypeInfo();

    bool isPlainVoid() const;
    bool isPlainInteger() const;

    bool isPointer() const;
    TypeInfo dereference() const;
    TypeInfo point() const;

    bool isVoidPointer() const;

    BasicType getBasicType() const;
    std::string getExtendedType() const;
private:
    BasicType basicType;
    std::string extendedType;
};

} /* namespace ast */

#endif /* TYPEINFO_H_ */
