#ifndef NUMERICTYPE_H_
#define NUMERICTYPE_H_

#include "BaseType.h"

namespace ast {

class NumericType: public BaseType {
public:
    static NumericType createCharacter();
    static NumericType createInteger();
    static NumericType createFloat();

    std::unique_ptr<BaseType> clone() const override;

    bool canConvertTo(const BaseType& otherType) const noexcept override;

    bool canConvertTo(const NumericType&) const noexcept override;
    bool canConvertTo(const Void&) const noexcept override;
    bool canConvertTo(const Function&) const noexcept override;

    std::string toString() const override;

private:
    bool isEqual(const BaseType& otherType) const noexcept override;

    NumericType(int sizeOrder);
};

} /* namespace ast */

#endif /* NUMERICTYPE_H_ */
