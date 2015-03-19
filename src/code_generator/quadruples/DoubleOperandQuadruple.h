#ifndef DOUBLEOPERANDQUADRUPLE_H_
#define DOUBLEOPERANDQUADRUPLE_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class DoubleOperandQuadruple: public Quadruple {
public:
    DoubleOperandQuadruple(Value leftOperand, Value rightOperand, Value result);
    virtual ~DoubleOperandQuadruple() = default;

    Value getLeftOperand() const;
    Value getRightOperand() const;
    Value getResult() const;

private:
    Value leftOperand;
    Value rightOperand;
    Value result;
};

} /* namespace code_generator */

#endif /* DOUBLEOPERANDQUADRUPLE_H_ */
