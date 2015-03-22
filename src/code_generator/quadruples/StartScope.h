#ifndef STARTSCOPE_H_
#define STARTSCOPE_H_

#include <vector>

#include "../Value.h"
#include "Quadruple.h"

namespace codegen {

class StartScope: public Quadruple {
public:
    StartScope(std::vector<Value> values, std::vector<Value> arguments);
    virtual ~StartScope() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::vector<Value> getValues() const;
    std::vector<Value> getArguments() const;

private:
    void print(std::ostream& stream) const override;

    std::vector<Value> values;
    std::vector<Value> arguments;
};

} /* namespace code_generator */

#endif /* STARTSCOPE_H_ */
