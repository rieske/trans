#include "FunctionType.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace ast {

FunctionType::FunctionType(std::unique_ptr<FundamentalType> returnType, std::vector<std::unique_ptr<FundamentalType>> argumentTypes) :
        returnType { std::move(returnType) },
        argumentTypes { std::move(argumentTypes) }
{
}

FunctionType::FunctionType(const FunctionType& rhs) :
        FundamentalType(rhs),
        returnType { rhs.returnType->clone() }
{
    for (auto& argumentType : rhs.argumentTypes) {
        argumentTypes.push_back(std::unique_ptr<FundamentalType> { argumentType->clone() });
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

const FundamentalType& FunctionType::getReturnType() const {
    return *returnType;
}

const std::vector<std::unique_ptr<FundamentalType> >& FunctionType::getArgumentTypes() const {
    return argumentTypes;
}

std::string FunctionType::toString() const {
    std::ostringstream stringRepresentation;
    stringRepresentation << "function (";
    for (const auto& argument : argumentTypes) {
        stringRepresentation << argument->toString() << ", ";
    }
    stringRepresentation << ") returning " << returnType->toString();
    return stringRepresentation.str();
}

FunctionType* FunctionType::clone() const {
    return new FunctionType { *this };
}

}

int ast::FunctionType::getSizeInBytes() const {
    return 0;
}
