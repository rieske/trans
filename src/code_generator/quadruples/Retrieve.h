#ifndef RETRIEVE_H_
#define RETRIEVE_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class Retrieve: public Quadruple {
public:
    Retrieve(Value result);
    virtual ~Retrieve() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getResult() const;

private:
    Value result;
};

} /* namespace code_generator */

#endif /* RETRIEVE_H_ */
