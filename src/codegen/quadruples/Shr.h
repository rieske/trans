#ifndef _SHR_H_
#define _SHR_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Shr: public DoubleOperandQuadruple {
public:
    Shr(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Shr() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // _SHR_H_
