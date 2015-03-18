#ifndef ADDRESSOF_H_
#define ADDRESSOF_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class AddressOf: public Quadruple {
public:
    AddressOf(Value argument, Value result);
    virtual ~AddressOf() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getArgument() const;
    Value getResult() const;

private:
    Value argument;
    Value result;
};

} /* namespace code_generator */

#endif /* ADDRESSOF_H_ */
