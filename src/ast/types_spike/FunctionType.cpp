#include "FunctionType.h"

#include <algorithm>

#include "StoredType.h"

namespace ast {

FunctionType::FunctionType(std::unique_ptr<StoredType> returnType, std::vector<std::unique_ptr<StoredType>> argumentTypes) :
        returnType { std::move(returnType) },
        argumentTypes { std::move(argumentTypes) }
{
}

FunctionType::FunctionType(const FunctionType& rhs) :
        FundamentalType(rhs),
        returnType { rhs.returnType->clone() }
{
    for (auto& argumentType : rhs.argumentTypes) {
        argumentTypes.push_back(std::unique_ptr<StoredType> { argumentType->clone() });
    }
}

FunctionType::FunctionType(FunctionType&& rhs) :
        FundamentalType(rhs),
        returnType { std::move(rhs.returnType) },
        argumentTypes { std::move(rhs.argumentTypes) }
{
}

FunctionType& FunctionType::operator=(const FunctionType& rhs) {
    FunctionType copy { rhs };
    this->returnType = std::move(copy.returnType);
    this->argumentTypes = std::move(copy.argumentTypes);
    return *this;
}

FunctionType& FunctionType::operator=(FunctionType&& rhs) {
    this->returnType = std::move(rhs.returnType);
    this->argumentTypes = std::move(rhs.argumentTypes);
    return *this;
}

FunctionType* FunctionType::clone() const {
    return new FunctionType { *this };
}

}

