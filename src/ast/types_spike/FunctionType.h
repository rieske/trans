#ifndef FUNCTIONTYPE_H_
#define FUNCTIONTYPE_H_

#include <memory>
#include <vector>

#include "FundamentalType.h"

namespace ast {

class StoredType;

class FunctionType: public FundamentalType {
public:
    FunctionType(std::unique_ptr<StoredType> returnType, std::vector<std::unique_ptr<StoredType>> argumentTypes);
    FunctionType(const FunctionType& rhs);
    FunctionType(FunctionType&& rhs);
    FunctionType& operator=(const FunctionType& rhs);
    FunctionType& operator=(FunctionType&& rhs);
    ~FunctionType() = default;

private:
    FunctionType* clone() const override;

    std::unique_ptr<StoredType> returnType;
    std::vector<std::unique_ptr<StoredType>> argumentTypes;
};

} /* namespace ast */

#endif /* FUNCTIONTYPE_H_ */
