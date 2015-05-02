#ifndef STARTPROCEDURE_H_
#define STARTPROCEDURE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class StartProcedure: public Quadruple {
public:
    StartProcedure(std::string name);
    virtual ~StartProcedure() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getName() const;

private:
    void print(std::ostream& stream) const override;

    std::string name;
};

} /* namespace code_generator */

#endif /* STARTPROCEDURE_H_ */
