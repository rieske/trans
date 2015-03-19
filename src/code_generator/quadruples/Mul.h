#ifndef MUL_H_
#define MUL_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Mul: public DoubleOperandQuadruple {
public:
    Mul(Value leftOperand, Value rightOperand, Value result);
    virtual ~Mul() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* MUL_H_ */
