#ifndef ARGUMENT_H_
#define ARGUMENT_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

class Argument: public Quadruple {
public:
    Argument(std::string argumentName);
    virtual ~Argument() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getArgumentName() const;

private:
    std::string argumentName;
};

} /* namespace code_generator */

#endif /* ARGUMENT_H_ */
