#ifndef BITWISENOT_H_
#define BITWISENOT_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class BitwiseNot: public SingleOperandQuadruple {
public:
    BitwiseNot(std::string operand, std::string result);
    virtual ~BitwiseNot() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // BITWISENOT_H_
