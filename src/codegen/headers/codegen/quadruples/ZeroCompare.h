#ifndef ZEROCOMPARE_H_
#define ZEROCOMPARE_H_

#include <string>
#include "Quadruple.h"

namespace codegen {

class ZeroCompare: public Quadruple {
public:
    ZeroCompare(std::string symbolName);
    virtual ~ZeroCompare() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getSymbolName() const;

private:
    void print(std::ostream& stream) const override;

    std::string symbolName;
};

} /* namespace code_generator */

#endif /* ZEROCOMPARE_H_ */
