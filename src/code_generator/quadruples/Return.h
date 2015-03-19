#ifndef RETURN_H_
#define RETURN_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class Return: public Quadruple {
public:
    Return(Value returnValue);
    virtual ~Return() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getReturnValue() const;

private:
    Value returnValue;
};

} /* namespace code_generator */

#endif /* RETURN_H_ */
