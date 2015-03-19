#ifndef ADDRESSOF_H_
#define ADDRESSOF_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class AddressOf: public SingleOperandQuadruple {
public:
    AddressOf(Value operand, Value result);
    virtual ~AddressOf() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* ADDRESSOF_H_ */
