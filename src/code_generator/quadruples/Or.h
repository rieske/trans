#ifndef OR_H_
#define OR_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Or: public DoubleOperandQuadruple {
public:
    Or(Value leftOperand, Value rightOperand, Value result);
    virtual ~Or() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* OR_H_ */
