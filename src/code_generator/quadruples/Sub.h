/*
 * Sub.h
 *
 *  Created on: Mar 19, 2015
 *      Author: rieske
 */

#ifndef SUB_H_
#define SUB_H_

#include "DoubleOperandQuadruple.h"

namespace code_generator {

class Sub: public DoubleOperandQuadruple {
public:
    Sub(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Sub() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} /* namespace code_generator */

#endif /* SUB_H_ */
