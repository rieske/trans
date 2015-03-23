#ifndef VOIDTYPE_H_
#define VOIDTYPE_H_

#include "FundamentalType.h"

namespace ast {

class VoidType: public FundamentalType {
public:
    VoidType* clone() const override;

    std::string toString() const override;

    bool isVoid() const override;

    int getSizeInBytes() const override;

};

} /* namespace ast */

#endif /* VOIDTYPE_H_ */
