#ifndef ASSIGN_H_
#define ASSIGN_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class Assign: public SingleOperandQuadruple {
public:
    Assign(Value operand, Value result);
    virtual ~Assign() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* ASSIGN_H_ */
