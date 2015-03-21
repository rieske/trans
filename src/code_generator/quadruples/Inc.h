#ifndef INC_H_
#define INC_H_

#include <string>

#include "SingleOperandQuadruple.h"

namespace code_generator {

class Inc: public Quadruple {
public:
    Inc(std::string operand);
    virtual ~Inc() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;

private:
    std::string operandName;
};

} /* namespace code_generator */

#endif /* INC_H_ */
