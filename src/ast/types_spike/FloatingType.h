#ifndef FLOATINGTYPE_H_
#define FLOATINGTYPE_H_

#include "StoredType.h"

namespace ast {

class FloatingType: public StoredType {
public:
    enum class Floating {
        FLOAT, DOUBLE, LONG_DOUBLE
    };

    FloatingType(Floating type);

    FloatingType* clone() const override;

private:
    Floating type;
};

} /* namespace ast */

#endif /* FLOATINGTYPE_H_ */
