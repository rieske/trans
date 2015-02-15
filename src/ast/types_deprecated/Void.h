#ifndef VOID_H_
#define VOID_H_

#include "BaseType.h"

namespace ast {

class Void: public BaseType {
public:
    Void();
    virtual ~Void();

    std::unique_ptr<BaseType> clone() const override;

    bool canConvertTo(const BaseType& otherType) const noexcept override;

    bool canConvertTo(const NumericType&) const noexcept override;
    bool canConvertTo(const Void&) const noexcept override;
    bool canConvertTo(const Function&) const noexcept override;

    std::string toString() const override;

private:
    bool isEqual(const BaseType& otherType) const noexcept override;
};

} /* namespace ast */

#endif /* VOID_H_ */
