#ifndef UNARYMINUS_H_
#define UNARYMINUS_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class UnaryMinus: public SingleOperandQuadruple {
public:
    UnaryMinus(std::string operand, std::string result);
    virtual ~UnaryMinus() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* UNARYMINUS_H_ */
