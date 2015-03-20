#ifndef INPUT_H_
#define INPUT_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

class Input: public Quadruple {
public:
    Input(std::string inputSymbolName);
    virtual ~Input() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getInputSymbolName() const;

private:
    std::string inputSymbolName;
};

} /* namespace code_generator */

#endif /* INPUT_H_ */
