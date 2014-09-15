#ifndef TYPEINFO_H_
#define TYPEINFO_H_

#include <string>

#include "BasicType.h"

namespace ast {

class TypeInfo {
public:
    TypeInfo(BasicType basicType, int dereferenceCount = 0);
    virtual ~TypeInfo();

    bool isPlainVoid() const;
    bool isPlainInteger() const;

    bool isPointer() const;
    TypeInfo dereference() const;
    TypeInfo point() const;

    bool isVoidPointer() const;

    BasicType getBasicType() const;
    int getDereferenceCount() const;

    bool operator==(const TypeInfo& rhs) const;

private:
    BasicType basicType;
    int dereferenceCount;
};

} /* namespace ast */

#endif /* TYPEINFO_H_ */
