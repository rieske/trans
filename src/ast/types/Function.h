#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <vector>

#include "BaseType.h"
#include "Type.h"

namespace ast {

class Function: public BaseType {
public:
    Function(Type returnType, std::vector<Type> argumentTypes = { });
    Function(const Function& rhs);
    virtual ~Function();

    Function& operator=(const Function& assignFrom);

    std::unique_ptr<BaseType> clone() const override;

    bool canConvertTo(const BaseType& otherType) const noexcept override;

    bool canConvertTo(const NumericType&) const noexcept override;
    bool canConvertTo(const Void&) const noexcept override;
    bool canConvertTo(const Function& anotherFunction) const noexcept override;

    const std::vector<Type>& getArgumentTypes() const;
    const Type& getReturnType() const;

    std::string toString() const override;
private:
    bool isEqual(const BaseType& otherType) const noexcept override;
    bool isEqual(const Function& otherFunction) const noexcept override;

    Type returnType;
    std::vector<Type> argumentTypes;
};

} /* namespace ast */

#endif /* FUNCTION_H_ */
