#ifndef STARTSCOPE_H_
#define STARTSCOPE_H_

#include <vector>

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class StartScope: public Quadruple {
public:
    StartScope(std::vector<Value> values);
    virtual ~StartScope() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::vector<Value> getValues() const;

private:
    std::vector<Value> values;
};

} /* namespace code_generator */

#endif /* STARTSCOPE_H_ */
