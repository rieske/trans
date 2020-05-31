#ifndef RETURN_H_
#define RETURN_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Return: public Quadruple {
public:
    Return(std::string returnSymbolName);
    virtual ~Return() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    bool transfersControl() const override;

    std::string getReturnSymbolName() const;

private:
    void print(std::ostream& stream) const override;

    std::string returnSymbolName;
};

} /* namespace code_generator */

#endif /* RETURN_H_ */
