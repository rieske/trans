#ifndef INTEGRALTYPE_H_
#define INTEGRALTYPE_H_

#include "StoredType.h"

namespace ast {

class IntegralType: public StoredType {
public:
    enum class Integral {
        SIGNED_CHAR, UNSIGNED_CHAR, SIGNED_SHORT, UNSIGNED_SHORT, SIGNED_INT, UNSIGNED_INT, SIGNED_LONG, UNSIGNED_LONG
    };

    IntegralType(Integral type);

    IntegralType* clone() const override;

private:
    Integral type;
};

} /* namespace ast */

#endif /* INTEGRALTYPE_H_ */
