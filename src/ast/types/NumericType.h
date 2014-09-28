#ifndef NUMERICTYPE_H_
#define NUMERICTYPE_H_

#include "BaseType.h"

namespace ast {

class NumericType: public BaseType {
public:
    static NumericType createCharacter();
    static NumericType createInteger();
    static NumericType createFloat();

    std::unique_ptr<BaseType> clone() const;

    const NumericType& promote(const BaseType& otherType) const override;
    const NumericType& promote(const NumericType& otherType) const override;
    const Void& promote(const Void&) const override;

private:
    NumericType(int sizeOrder);
};

} /* namespace ast */

#endif /* NUMERICTYPE_H_ */
