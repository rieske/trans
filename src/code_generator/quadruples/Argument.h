#ifndef ARGUMENT_H_
#define ARGUMENT_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class Argument: public Quadruple {
public:
    Argument(Value argument);
    virtual ~Argument() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getArgument() const;

private:
    Value argument;
};

} /* namespace code_generator */

#endif /* ARGUMENT_H_ */
