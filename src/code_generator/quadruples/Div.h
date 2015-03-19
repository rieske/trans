#ifndef DIV_H_
#define DIV_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Div: public DoubleOperandQuadruple {
public:
    Div(Value leftOperand, Value rightOperand, Value result);
    virtual ~Div() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* DIV_H_ */
