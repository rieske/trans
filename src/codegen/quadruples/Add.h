#ifndef ADD_H_
#define ADD_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Add: public DoubleOperandQuadruple {
public:
    Add(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Add() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // ADD_H_
