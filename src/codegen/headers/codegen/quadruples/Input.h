#ifndef INPUT_H_
#define INPUT_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Input: public Quadruple {
public:
    Input(std::string inputSymbolName);
    virtual ~Input() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getInputSymbolName() const;

private:
    void print(std::ostream& stream) const override;

    std::string inputSymbolName;
};

} /* namespace code_generator */

#endif /* INPUT_H_ */
