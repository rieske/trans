#ifndef INC_H_
#define INC_H_

#include <string>

#include "SingleOperandQuadruple.h"

namespace codegen {

class Inc: public Quadruple {
public:
    // amount is 1 for integers; sizeof(pointee) (word-aligned) for pointer ++.
    Inc(std::string operand, int amount = 1);
    virtual ~Inc() = default;

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

#endif // INC_H_
