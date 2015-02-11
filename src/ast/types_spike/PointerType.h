#ifndef POINTERTYPE_H_
#define POINTERTYPE_H_

#include <memory>

#include "StoredType.h"

namespace ast {

class PointerType: public StoredType {
public:
    PointerType(std::unique_ptr<FundamentalType> to, bool _const, bool _volatile);
    PointerType(const PointerType& rhs);
    PointerType(PointerType&& rhs);
    PointerType& operator=(const PointerType& rhs);
    PointerType& operator=(PointerType&& rhs);
    ~PointerType() = default;

private:
    std::unique_ptr<FundamentalType> pointsTo;

    bool _const;
    bool _volatile;
};

}
#endif /* POINTERTYPE_H_ */
