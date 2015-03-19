#ifndef INC_H_
#define INC_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class Inc: public SingleOperandQuadruple {
public:
    Inc(Value operand, Value result);
    virtual ~Inc() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* INC_H_ */
