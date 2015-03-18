#ifndef DEREFERENCE_H_
#define DEREFERENCE_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class Dereference: public Quadruple {
public:
    Dereference(Value argument, Value result);
    virtual ~Dereference() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getArgument() const;
    Value getResult() const;

private:
    Value argument;
    Value result;
};

} /* namespace code_generator */

#endif /* DEREFERENCE_H_ */
