#ifndef STARTSCOPE_H_
#define STARTSCOPE_H_

#include "Quadruple.h"

namespace code_generator {

class StartScope: public Quadruple {
public:
    StartScope(int scopeSize);
    virtual ~StartScope() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    int getScopeSize() const;

private:
    int scopeSize;
};

} /* namespace code_generator */

#endif /* STARTSCOPE_H_ */
