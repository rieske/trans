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

    bool canConvertTo(const BaseType& otherType) const noexcept override;

    bool canConvertTo(const NumericType&) const noexcept override;
    bool canConvertTo(const Void&) const noexcept override;
    bool canConvertTo(const Function& anotherFunction) const noexcept override;

    std::string toString() const override;
private:
    bool isEqual(const BaseType& otherType) const noexcept override;
    bool isEqual(const Function& otherFunction) const noexcept override;

    std::vector<Type> argumentTypes;
};

} /* namespace ast */

#endif /* FUNCTION_H_ */
