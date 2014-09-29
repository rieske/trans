#ifndef VOID_H_
#define VOID_H_

#include "BaseType.h"

namespace ast {

class Void: public BaseType {
public:
    Void();
    virtual ~Void();

    std::unique_ptr<BaseType> clone() const override;

    const Void& convertTo(const BaseType& otherType) const override;
    const NumericType& convertFrom(const NumericType&) const override;
    const Void& convertFrom(const Void&) const override;
    const Function& convertFrom(const Function&) const override;

    std::string toString() const override;

protected:
    bool isEqual(const BaseType& otherType) const noexcept override;
};

} /* namespace ast */

#endif /* VOID_H_ */
