#ifndef ENDPROCEDURE_H_
#define ENDPROCEDURE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class EndProcedure: public Quadruple {
public:
    EndProcedure(std::string name);
    virtual ~EndProcedure() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getName() const;

private:
    void print(std::ostream& stream) const override;

    std::string name;
};

} // namespace codegen

#endif // ENDPROCEDUbE_H_
