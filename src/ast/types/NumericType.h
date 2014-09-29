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

    const NumericType& convertTo(const BaseType& otherType) const override;
    const NumericType& convertFrom(const NumericType& otherType) const override;
    const Void& convertFrom(const Void&) const override;
    const Function& convertFrom(const Function&) const override;

    std::string toString() const override;

private:
    bool isEqual(const BaseType& otherType) const noexcept override;

    NumericType(int sizeOrder);
};

} /* namespace ast */

#endif /* NUMERICTYPE_H_ */
