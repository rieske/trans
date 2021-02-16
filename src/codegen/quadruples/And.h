#ifndef AND_H_
#define AND_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class And: public DoubleOperandQuadruple {
public:
    And(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~And() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // AND_H_
