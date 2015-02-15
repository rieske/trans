#include "PointerType.h"

#include <algorithm>

namespace ast {

PointerType::PointerType(std::unique_ptr<FundamentalType> to, std::set<TypeQualifier> qualifiers) :
        pointsTo { std::move(to) },
        qualifiers { qualifiers }
{
}

PointerType::PointerType(const PointerType& rhs) :
        FundamentalType(rhs),
        pointsTo { rhs.pointsTo->clone() },
        qualifiers { rhs.qualifiers }
{
}

PointerType::PointerType(PointerType&& rhs) :
        FundamentalType(rhs),
        pointsTo { std::move(rhs.pointsTo) },
        qualifiers { std::move(rhs.qualifiers) }
{
}

PointerType& PointerType::operator=(const PointerType& rhs) {
    this->pointsTo.reset(rhs.pointsTo->clone());
    this->qualifiers = rhs.qualifiers;
    return *this;
}

PointerType& PointerType::operator=(PointerType&& rhs) {
    this->pointsTo = std::move(rhs.pointsTo);
    this->qualifiers = std::move(rhs.qualifiers);
    return *this;
}

PointerType* PointerType::clone() const {
    return new PointerType { *this };
}

bool PointerType::isPointer() const {
    return true;
}

std::string PointerType::toString() const {
    return "pointer to " + pointsTo->toString();
}

std::unique_ptr<FundamentalType> PointerType::dereference() const {
    return std::unique_ptr<FundamentalType> { pointsTo->clone() };
}

} /* namespace ast */

