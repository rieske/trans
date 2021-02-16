#ifndef INC_H_
#define INC_H_

#include <string>

#include "SingleOperandQuadruple.h"

namespace codegen {

class Inc: public Quadruple {
public:
    Inc(std::string operand);
    virtual ~Inc() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;

private:
    void print(std::ostream& stream) const override;

    std::string operandName;
};

} // namespace codegen

#endif // INC_H_
