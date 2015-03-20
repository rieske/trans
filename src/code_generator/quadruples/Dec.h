#ifndef DEC_H_
#define DEC_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

class Dec: public SingleOperandQuadruple {
public:
    Dec(std::string operand, std::string result);
    virtual ~Dec() = default;

    void generateCode(AssemblyGenerator& generator) const override;
};

} /* namespace code_generator */

#endif /* DEC_H_ */
