#ifndef VOID_H_
#define VOID_H_

#include "BaseType.h"

namespace ast {

class Void: public BaseType {
public:
    Void();
    virtual ~Void();

    std::unique_ptr<BaseType> clone() const;

    const Void& promote(const BaseType& otherType) const override;
    const NumericType& promote(const NumericType&) const override;
    const Void& promote(const Void&) const override;
};

} /* namespace ast */

#endif /* VOID_H_ */
