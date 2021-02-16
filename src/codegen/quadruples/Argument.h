#ifndef ARGUMENT_H_
#define ARGUMENT_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Argument: public Quadruple {
public:
    Argument(std::string argumentName);
    virtual ~Argument() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getArgumentName() const;

private:
    void print(std::ostream& stream) const override;

    std::string argumentName;
};

} // namespace codegen

#endif // ARGUMENT_H_
