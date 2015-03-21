#ifndef DIV_H_
#define DIV_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Div: public DoubleOperandQuadruple {
public:
    Div(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Div() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* DIV_H_ */
