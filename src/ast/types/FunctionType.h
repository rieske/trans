#ifndef FUNCTIONTYPE_H_
#define FUNCTIONTYPE_H_

#include <memory>
#include <vector>

#include "FundamentalType.h"

namespace ast {

class FunctionType: public FundamentalType {
public:
    FunctionType(std::unique_ptr<FundamentalType> returnType, std::vector<std::unique_ptr<FundamentalType>> argumentTypes);
    FunctionType(const FunctionType& rhs);
    FunctionType(FunctionType&& rhs);
    FunctionType& operator=(const FunctionType& rhs);
    FunctionType& operator=(FunctionType&& rhs);
    ~FunctionType() = default;

    std::string toString() const override;

    const FundamentalType& getReturnType() const;
    const std::vector<std::unique_ptr<FundamentalType>>& getArgumentTypes() const;

    int getSizeInBytes() const override;

private:
    FunctionType* clone() const override;

    std::unique_ptr<FundamentalType> returnType;
    std::vector<std::unique_ptr<FundamentalType>> argumentTypes;
};

} /* namespace ast */

#endif /* FUNCTIONTYPE_H_ */
