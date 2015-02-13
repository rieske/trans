#ifndef VOIDTYPE_H_
#define VOIDTYPE_H_

#include "StoredType.h"

namespace ast {

class VoidType: public StoredType {
public:
    VoidType* clone() const override;
};

} /* namespace ast */

#endif /* VOIDTYPE_H_ */
