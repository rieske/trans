#ifndef ZEROCOMPARE_H_
#define ZEROCOMPARE_H_

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class ZeroCompare: public Quadruple {
public:
    ZeroCompare(Value argument);
    virtual ~ZeroCompare() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    Value getArgument() const;

private:
    Value argument;
};

} /* namespace code_generator */

#endif /* ZEROCOMPARE_H_ */
