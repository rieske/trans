#ifndef MUL_H_
#define MUL_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Mul: public DoubleOperandQuadruple {
public:
    Mul(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Mul() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} /* namespace code_generator */

#endif /* MUL_H_ */
