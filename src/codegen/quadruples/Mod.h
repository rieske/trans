#ifndef MOD_H_
#define MOD_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Mod: public DoubleOperandQuadruple {
public:
    Mod(std::string leftOperand, std::string rightOperand, std::string result);
    virtual ~Mod() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} /* namespace code_generator */

#endif /* MOD_H_ */
