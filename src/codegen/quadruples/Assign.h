#ifndef ASSIGN_H_
#define ASSIGN_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class Assign: public SingleOperandQuadruple {
public:
    Assign(std::string operand, std::string result);
    virtual ~Assign() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // ASSIGN_H_
