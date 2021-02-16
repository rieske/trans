#ifndef DIV_H_
#define DIV_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Div: public DoubleOperandQuadruple {
public:
    Div(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Div() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // DIV_H_
