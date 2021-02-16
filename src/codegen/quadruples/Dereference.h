#ifndef DEREFERENCE_H_
#define DEREFERENCE_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class Dereference: public SingleOperandQuadruple {
public:
    Dereference(std::string operand, std::string lvalue, std::string result);
    virtual ~Dereference() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getLvalue() const;

private:
    void print(std::ostream& stream) const override;

    std::string lvalue;
};

} // namespace codegen

#endif // DEREFERENCE_H_
