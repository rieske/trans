#ifndef INC_H_
#define INC_H_

#include <string>

#include "SingleOperandQuadruple.h"

namespace codegen {

// step is in addressable units for the value: 1 for scalar ++, sizeof(*p) bytes for pointer ++.
class Inc: public Quadruple {
public:
    Inc(std::string operand, int step = 1);
    virtual ~Inc() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;
    int getStep() const { return step; }

private:
    void print(std::ostream& stream) const override;

    std::string operandName;
    int step;
};

} // namespace codegen

#endif // INC_H_
