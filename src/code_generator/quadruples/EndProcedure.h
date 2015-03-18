#ifndef ENDPROCEDURE_H_
#define ENDPROCEDURE_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

class EndProcedure: public Quadruple {
public:
    EndProcedure(std::string name);
    virtual ~EndProcedure() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getName() const;

private:
    std::string name;
};

} /* namespace code_generator */

#endif /* ENDPROCEDURE_H_ */
