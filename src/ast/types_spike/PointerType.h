#ifndef POINTERTYPE_H_
#define POINTERTYPE_H_

#include <memory>
#include <set>

#include "../TypeQualifier.h"
#include "StoredType.h"

namespace ast {

class PointerType: public StoredType {
public:
    PointerType(std::unique_ptr<FundamentalType> to, std::set<TypeQualifier> qualifiers);
    PointerType(const PointerType& rhs);
    PointerType(PointerType&& rhs);
    PointerType& operator=(const PointerType& rhs);
    PointerType& operator=(PointerType&& rhs);
    ~PointerType() = default;

    PointerType* clone() const override;

private:
    std::unique_ptr<FundamentalType> pointsTo;

    std::set<TypeQualifier> qualifiers;
};

}
#endif /* POINTERTYPE_H_ */
