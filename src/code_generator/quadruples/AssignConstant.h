#ifndef ASSIGNCONSTANT_H_
#define ASSIGNCONSTANT_H_

#include <string>

#include "../Value.h"
#include "Quadruple.h"

namespace code_generator {

class AssignConstant: public Quadruple {
public:
    AssignConstant(std::string constant, std::string result);
    virtual ~AssignConstant() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getConstant() const;
    std::string getResult() const;

private:
    std::string constant;
    std::string result;
};

} /* namespace code_generator */

#endif /* ASSIGNCONSTANT_H_ */
