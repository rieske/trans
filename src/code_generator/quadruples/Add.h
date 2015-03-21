#ifndef ADD_H_
#define ADD_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Add: public DoubleOperandQuadruple {
public:
    Add(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Add() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* ADD_H_ */
