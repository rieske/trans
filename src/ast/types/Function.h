#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <vector>

#include "BaseType.h"
#include "Type.h"

namespace ast {

class Function: public BaseType {
public:
    Function(std::vector<Type> argumentTypes = { });
    virtual ~Function();

    std::unique_ptr<BaseType> clone() const override;

    const BaseType& convertTo(const BaseType& otherType) const override;
    const NumericType& convertFrom(const NumericType&) const override;
    const Void& convertFrom(const Void&) const override;
    const Function& convertFrom(const Function& anotherFunction) const override;

    std::string toString() const override;
private:
    bool isEqual(const BaseType& otherType) const noexcept override;
    bool isEqual(const Function& otherFunction) const noexcept override;

    std::vector<Type> argumentTypes;
};

} /* namespace ast */

#endif /* FUNCTION_H_ */
