#include "ArrayType.h"

#include <algorithm>

namespace ast {

ArrayType::ArrayType(std::unique_ptr<StoredType> elementType, std::size_t size) :
        elementType { std::move(elementType) },
        size { size }
{
}

ArrayType::ArrayType(const ArrayType& rhs) :
        StoredType(rhs),
        elementType { rhs.elementType->clone() },
        size { rhs.size }
{
}

ArrayType::ArrayType(ArrayType&& rhs) :
        StoredType(rhs),
        elementType(std::move(rhs.elementType)),
        size { std::move(rhs.size) }
{
}

ArrayType& ArrayType::operator=(const ArrayType& rhs) {
    this->elementType.reset(rhs.elementType->clone());
    this->size = rhs.size;
    return *this;
}

ArrayType& ArrayType::operator=(ArrayType&& rhs) {
    this->elementType = std::move(rhs.elementType);
    this->size = std::move(rhs.size);
    return *this;
}

ArrayType* ArrayType::clone() const {
    return new ArrayType { *this };
}

} /* namespace ast */
