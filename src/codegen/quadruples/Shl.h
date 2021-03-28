#ifndef _SHL_H_
#define _SHL_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Shl: public DoubleOperandQuadruple {
public:
    Shl(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Shl() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // _SHL_H_
