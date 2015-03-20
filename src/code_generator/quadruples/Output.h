#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include "Quadruple.h"

namespace code_generator {

class Output: public Quadruple {
public:
    Output(std::string outputSymbolName);
    virtual ~Output() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOutputSymbolName() const;

private:
    std::string outputSymbolName;
};

} /* namespace code_generator */

#endif /* OUTPUT_H_ */
