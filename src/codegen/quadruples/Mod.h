#ifndef MOD_H_
#define MOD_H_

#include "DoubleOperandQuadruple.h"

namespace codegen {

class Mod: public DoubleOperandQuadruple {
public:
    Mod(std::string leftOperand, std::string rightOperand, std::string result, bool unsignedMod = false);
    virtual ~Mod() = default;

    void generateCode(AssemblyGenerator& generator) const override;
    bool isUnsigned() const;

private:
    void print(std::ostream& stream) const override;
    bool unsignedMod;
};

} // namespace codegen

#endif // MOD_H_
