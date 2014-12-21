#ifndef BASETYPE_H_
#define BASETYPE_H_

#include <memory>
#include <stdexcept>
#include <string>

namespace ast {
class NumericType;
class Void;
class Function;
} /* namespace ast */

namespace ast {

class TypeConversionException: public std::runtime_error {
public:
    TypeConversionException(const std::string& message) :
            std::runtime_error { message } {
    }
};

class BaseType {
public:
    static const NumericType CHARACTER;
    static const NumericType INTEGER;
    static const NumericType FLOAT;
    static const Void VOID;

    static std::unique_ptr<BaseType> newCharacter();
    static std::unique_ptr<BaseType> newInteger();
    static std::unique_ptr<BaseType> newFloat();
    static std::unique_ptr<BaseType> newVoid();

    virtual ~BaseType();

    virtual std::unique_ptr<BaseType> clone() const = 0;

    virtual bool canConvertTo(const BaseType& otherType) const noexcept = 0;

    virtual bool canConvertTo(const NumericType& otherType) const noexcept = 0;
    virtual bool canConvertTo(const Void& otherType) const noexcept = 0;
    virtual bool canConvertTo(const Function& otherFunction) const noexcept = 0;

    virtual std::string toString() const = 0;

    bool operator==(const BaseType& rhs) const noexcept;
    bool operator!=(const BaseType& rhs) const noexcept;

    bool isEqual(const NumericType&) const noexcept;
    bool isEqual(const Void&) const noexcept;
    virtual bool isEqual(const Function&) const noexcept;
protected:
    virtual bool isEqual(const BaseType& otherType) const noexcept = 0;

    BaseType(int sizeOrder);

    const int sizeOrder;
};

} /* namespace ast */

#endif /* BASETYPE_H_ */
