#ifndef SINGLEOPERANDQUADRUPLE_H_
#define SINGLEOPERANDQUADRUPLE_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class SingleOperandQuadruple: public Quadruple {
public:
    SingleOperandQuadruple(Value operand, Value result);
    virtual ~SingleOperandQuadruple() = default;

    Value getOperand() const;
    Value getResult() const;

private:
    Value operand;
    Value result;
};

} /* namespace code_generator */

#endif /* SINGLEOPERANDQUADRUPLE_H_ */
