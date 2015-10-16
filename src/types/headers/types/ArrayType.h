#ifndef ARRAYTYPE_H_
#define ARRAYTYPE_H_

#include <cstddef>
#include <memory>

#include "FundamentalType.h"

namespace ast {

class ArrayType: public FundamentalType {
public:
    ArrayType(std::unique_ptr<FundamentalType> elementType, std::size_t size);
    ArrayType(const ArrayType& rhs);
    ArrayType(ArrayType&& rhs);
    ArrayType& operator=(const ArrayType& rhs);
    ArrayType& operator=(ArrayType&& rhs);
    ~ArrayType() = default;

    std::string toString() const override;

    bool isPointer() const override;
    std::unique_ptr<FundamentalType> dereference() const override;

    int getSizeInBytes() const override;

private:
    ArrayType* clone() const override;

    std::unique_ptr<FundamentalType> elementType;
    std::size_t size;
};

} /* namespace ast */

#endif /* ARRAYTYPE_H_ */
