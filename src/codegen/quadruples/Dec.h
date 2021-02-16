#ifndef DEC_H_
#define DEC_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Dec: public Quadruple {
public:
    Dec(std::string operandName);
    virtual ~Dec() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;

private:
    void print(std::ostream& stream) const override;

    std::string operandName;
};

} // namespace codegen

#endif // DEC_H_
