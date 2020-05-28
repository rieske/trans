#ifndef VOID_RETURN_H_
#define VOID_RETURN_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class VoidReturn: public Quadruple {
public:
    VoidReturn() = default;
    virtual ~VoidReturn() = default;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;
};

} /* namespace codegen */

#endif /* VOID_RETURN_H_ */
