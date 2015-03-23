#ifndef INTEGRALTYPE_H_
#define INTEGRALTYPE_H_

#include <memory>

#include "FundamentalType.h"

namespace ast {

class IntegralType: public FundamentalType {
public:
    enum class Integral {
        SIGNED_CHAR, UNSIGNED_CHAR, SIGNED_SHORT, UNSIGNED_SHORT, SIGNED_INT, UNSIGNED_INT, SIGNED_LONG, UNSIGNED_LONG
    };

    IntegralType(Integral type);
    static std::unique_ptr<IntegralType> newSignedInteger();

    std::string toString() const override;

    bool isNumeric() const override;

    IntegralType* clone() const override;

    int getSizeInBytes() const override;

private:
    Integral type;
};

} /* namespace ast */

#endif /* INTEGRALTYPE_H_ */
