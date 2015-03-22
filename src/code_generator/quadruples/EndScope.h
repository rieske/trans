#ifndef ENDSCOPE_H_
#define ENDSCOPE_H_

#include "Quadruple.h"

namespace code_generator {

class EndScope: public Quadruple {
public:
    EndScope(int scopeSize);
    virtual ~EndScope() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    int getScopeSize() const;

private:
    void print(std::ostream& stream) const override;

    int scopeSize;
};

} /* namespace code_generator */

#endif /* ENDSCOPE_H_ */
