#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include "Quadruple.h"

namespace codegen {

class Output: public Quadruple {
public:
    Output(std::string outputSymbolName);
    virtual ~Output() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOutputSymbolName() const;

private:
    void print(std::ostream& stream) const override;

    std::string outputSymbolName;
};

} /* namespace code_generator */

#endif /* OUTPUT_H_ */
