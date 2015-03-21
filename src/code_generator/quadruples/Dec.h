#ifndef DEC_H_
#define DEC_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

class Dec: public Quadruple {
public:
    Dec(std::string operandName);
    virtual ~Dec() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;

private:
    std::string operandName;
};

} /* namespace code_generator */

#endif /* DEC_H_ */
