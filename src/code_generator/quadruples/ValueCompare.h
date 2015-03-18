#ifndef VALUECOMPARE_H_
#define VALUECOMPARE_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class ValueCompare: public Quadruple {
public:
    ValueCompare(Value leftArg, Value rightArg);
    virtual ~ValueCompare() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getLeftArg() const;
    Value getRightArg() const;

private:
    Value leftArg;
    Value rightArg;
};

} /* namespace code_generator */

#endif /* VALUECOMPARE_H_ */
