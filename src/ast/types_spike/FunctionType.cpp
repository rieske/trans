#include "FunctionType.h"

#include <algorithm>

#include "StoredType.h"

namespace ast {

FunctionType::FunctionType(std::vector<std::unique_ptr<StoredType>> argumentTypes) :
        argumentTypes { std::move(argumentTypes) }
{
}

FunctionType::FunctionType(const FunctionType& rhs) :
        FundamentalType(rhs)
{
    for (auto& argumentType : rhs.argumentTypes) {
        argumentTypes.push_back(std::unique_ptr<StoredType> { argumentType->clone() });
    }
}

FunctionType::FunctionType(FunctionType&& rhs) :
        FundamentalType(rhs),
        argumentTypes { std::move(rhs.argumentTypes) }
{
}

FunctionType& FunctionType::operator=(const FunctionType& rhs) {
    FunctionType copy { rhs };
    this->argumentTypes = std::move(copy.argumentTypes);
    return *this;
}

FunctionType& FunctionType::operator=(FunctionType&& rhs) {
    this->argumentTypes = std::move(rhs.argumentTypes);
    return *this;
}

FunctionType* FunctionType::clone() const {
    return new FunctionType { *this };
}

}

