#ifndef LVALUEASSIGN_H_
#define LVALUEASSIGN_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class LvalueAssign: public SingleOperandQuadruple {
public:
    LvalueAssign(Value operand, Value result);
    virtual ~LvalueAssign() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* LVALUEASSIGN_H_ */
