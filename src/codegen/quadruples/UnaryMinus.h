#ifndef UNARYMINUS_H_
#define UNARYMINUS_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class UnaryMinus: public SingleOperandQuadruple {
public:
    UnaryMinus(std::string operand, std::string result);
    virtual ~UnaryMinus() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // UNARYMINUS_H_
