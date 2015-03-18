#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class Output: public Quadruple {
public:
    Output(Value outputValue);
    virtual ~Output() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getOutputValue() const;

private:
    Value outputValue;
};

} /* namespace code_generator */

#endif /* OUTPUT_H_ */
