#ifndef XOR_H_
#define XOR_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Xor: public DoubleOperandQuadruple {
public:
    Xor(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Xor() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* XOR_H_ */
