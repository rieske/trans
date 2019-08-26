#ifndef FLOATINGTYPE_H_
#define FLOATINGTYPE_H_

#include "FundamentalType.h"

namespace ast {

class FloatingType: public FundamentalType {
public:
    enum class Floating {
        FLOAT, DOUBLE, LONG_DOUBLE
    };

    FloatingType(Floating type);

    std::string toString() const override;

    bool isNumeric() const override;

    int getSizeInBytes() const override;

    FloatingType* clone() const override;

private:
    Floating type;
};

} /* namespace ast */

#endif /* FLOATINGTYPE_H_ */
