#ifndef UNARYNOT_H_
#define UNARYNOT_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class UnaryNot: public SingleOperandQuadruple {
public:
    UnaryNot(std::string operand, std::string result);
    virtual ~UnaryNot() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // UNARYNOT_H_
