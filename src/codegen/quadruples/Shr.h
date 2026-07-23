#ifndef _SHR_H_
#define _SHR_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Shr: public DoubleOperandQuadruple {
public:
    // logical=true => unsigned >> (SHR); logical=false => signed >> (SAR).
    Shr(std::string leftOperand, std::string rightOperand, std::string result, bool logical = false);
    virtual ~Shr() = default;

    void generateCode(AssemblyGenerator& generator) const override;
    bool isLogical() const;

private:
    void print(std::ostream& stream) const override;
    bool logical;
};

} // namespace codegen

#endif // _SHR_H_
