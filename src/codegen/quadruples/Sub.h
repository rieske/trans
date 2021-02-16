#ifndef SUB_H_
#define SUB_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Sub: public DoubleOperandQuadruple {
public:
    Sub(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Sub() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // SUB_H_
