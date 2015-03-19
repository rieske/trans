#ifndef MOD_H_
#define MOD_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Mod: public DoubleOperandQuadruple {
public:
    Mod(Value leftOperand, Value rightOperand, Value result);
    virtual ~Mod() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* MOD_H_ */
