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

class TypePromotionException: public std::runtime_error {
public:
    TypePromotionException(const std::string& message) :
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

    bool isInteger() const noexcept;
    virtual const BaseType& promote(const BaseType& otherType) const = 0;
    virtual const NumericType& promote(const NumericType& otherType) const = 0;
    virtual const Void& promote(const Void& otherType) const = 0;

    bool operator==(const BaseType& rhs) const noexcept;
    bool operator!=(const BaseType& rhs) const noexcept;
protected:
    BaseType(int sizeOrder);

    const int sizeOrder;
};

} /* namespace ast */

#endif /* BASETYPE_H_ */
