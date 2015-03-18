#ifndef INPUT_H_
#define INPUT_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class Input: public Quadruple {
public:
    Input(Value inputValue);
    virtual ~Input() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getInputValue() const;

private:
    Value inputValue;
};

} /* namespace code_generator */

#endif /* INPUT_H_ */
