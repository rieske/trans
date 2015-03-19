#ifndef DEREFERENCE_H_
#define DEREFERENCE_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class Dereference: public SingleOperandQuadruple {
public:
    Dereference(Value operand, Value result);
    virtual ~Dereference() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* DEREFERENCE_H_ */
