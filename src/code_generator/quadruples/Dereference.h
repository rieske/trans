#ifndef DEREFERENCE_H_
#define DEREFERENCE_H_

#include "SingleOperandQuadruple.h"

namespace code_generator {

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

} /* namespace code_generator */

#endif /* DEREFERENCE_H_ */
