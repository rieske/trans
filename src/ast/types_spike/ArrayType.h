#ifndef ARRAYTYPE_H_
#define ARRAYTYPE_H_

#include <cstddef>
#include <memory>

#include "StoredType.h"

namespace ast {

class ArrayType: public StoredType {
public:
    ArrayType(std::unique_ptr<StoredType> elementType, std::size_t size);
    ArrayType(const ArrayType& rhs);
    ArrayType(ArrayType&& rhs);
    ArrayType& operator=(const ArrayType& rhs);
    ArrayType& operator=(ArrayType&& rhs);
    ~ArrayType() = default;

private:
    ArrayType* clone() const override;

    std::unique_ptr<StoredType> elementType;
    std::size_t size;
};

} /* namespace ast */

#endif /* ARRAYTYPE_H_ */
