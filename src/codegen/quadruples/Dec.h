#ifndef DEC_H_
#define DEC_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Dec: public Quadruple {
public:
    // amount is 1 for integers; sizeof(pointee) (word-aligned) for pointer --.
    Dec(std::string operandName, int amount = 1);
    virtual ~Dec() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;
    int getAmount() const { return amount; }

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;

    std::string operandName;
    int amount;
};

} // namespace codegen

#endif // DEC_H_
