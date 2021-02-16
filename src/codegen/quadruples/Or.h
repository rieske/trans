#ifndef OR_H_
#define OR_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Or: public DoubleOperandQuadruple {
public:
    Or(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Or() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace code_generator

#endif // OR_H_
