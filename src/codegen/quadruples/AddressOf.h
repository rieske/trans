#ifndef ADDRESSOF_H_
#define ADDRESSOF_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class AddressOf: public SingleOperandQuadruple {
public:
    AddressOf(std::string operand, std::string result);
    virtual ~AddressOf() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // ADDRESSOF_H_
