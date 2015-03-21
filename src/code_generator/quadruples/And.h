#ifndef AND_H_
#define AND_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class And: public DoubleOperandQuadruple {
public:
    And(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~And() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* AND_H_ */
