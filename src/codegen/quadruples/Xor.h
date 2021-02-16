#ifndef XOR_H_
#define XOR_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Xor: public DoubleOperandQuadruple {
public:
    Xor(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Xor() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // XOR_H_
