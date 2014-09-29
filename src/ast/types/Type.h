#ifndef TYPE_H_
#define TYPE_H_

#include <memory>
#include <string>

namespace ast {
class BaseType;
} /* namespace ast */

namespace ast {

class Type {
public:
    Type(std::unique_ptr<BaseType> baseType, int dereferenceCount = 0);
    Type(const Type& copyFrom);
    virtual ~Type();

    Type& operator=(const Type& assignFrom);

    bool operator==(const Type& rhs) const noexcept;
    bool operator!=(const Type& rhs) const noexcept;

    bool isPointer() const noexcept;

    bool isPlainVoid() const noexcept;
    bool isPlainInteger() const noexcept;

    Type getAddressType() const;
    Type getTypePointedTo() const;

    std::string toString() const;

private:
    std::unique_ptr<BaseType> baseType;
    int dereferenceCount { 0 };
};

}

#endif /* TYPE_H_ */
