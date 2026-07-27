#ifndef DEC_H_
#define DEC_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// step is in addressable units for the value: 1 for scalar --, sizeof(*p) bytes for pointer --.
class Dec: public Quadruple {
public:
    Dec(std::string operandName, int step = 1);
    virtual ~Dec() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;
    int getStep() const { return step; }

private:
    void print(std::ostream& stream) const override;

    std::string operandName;
    int step;
};

} // namespace codegen

#endif // DEC_H_
