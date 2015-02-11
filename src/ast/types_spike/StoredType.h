#ifndef STOREDTYPE_H_
#define STOREDTYPE_H_

#include <memory>

#include "FundamentalType.h"

namespace ast {

class StoredType: public FundamentalType {
public:
    virtual ~StoredType() = default;

    virtual StoredType* clone() const = 0;

protected:
    StoredType() = default;
    StoredType(const StoredType& rhs) = default;
    StoredType(StoredType&& rhs) = default;
    StoredType& operator=(const StoredType& rhs) = default;
    StoredType& operator=(StoredType&& rhs) = default;
};

}

#endif /* STOREDTYPE_H_ */
