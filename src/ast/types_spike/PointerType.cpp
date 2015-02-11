#include "PointerType.h"

#include <algorithm>

namespace ast {

PointerType::PointerType(std::unique_ptr<FundamentalType> to, bool _const, bool _volatile) :
        pointsTo { std::move(to) },
        _const { _const },
        _volatile { _volatile }
{
}

PointerType::PointerType(const PointerType& rhs) :
        StoredType(rhs),
        pointsTo { rhs.pointsTo->clone() },
        _const { rhs._const },
        _volatile { rhs._volatile }
{
}

PointerType::PointerType(PointerType&& rhs) :
        StoredType(rhs),
        pointsTo { std::move(rhs.pointsTo) },
        _const { std::move(rhs._const) },
        _volatile { std::move(rhs._volatile) }
{
}

PointerType& PointerType::operator=(const PointerType& rhs) {
    this->pointsTo.reset(rhs.pointsTo->clone());
    this->_const = rhs._const;
    this->_volatile = rhs._volatile;
    return *this;
}

PointerType& PointerType::operator=(PointerType&& rhs) {
    this->pointsTo = std::move(rhs.pointsTo);
    this->_const = std::move(rhs._const);
    this->_volatile = std::move(rhs._volatile);
    return *this;
}

} /* namespace ast */
