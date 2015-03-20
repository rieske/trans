#ifndef SINGLEOPERANDQUADRUPLE_H_
#define SINGLEOPERANDQUADRUPLE_H_

#include <string>
#include "Quadruple.h"

namespace code_generator {

class SingleOperandQuadruple: public Quadruple {
public:
    SingleOperandQuadruple(std::string operand, std::string result);
    virtual ~SingleOperandQuadruple() = default;

    std::string getOperand() const;
    std::string getResult() const;

private:
    std::string operand;
    std::string result;
};

} /* namespace code_generator */

#endif /* SINGLEOPERANDQUADRUPLE_H_ */
