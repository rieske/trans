#ifndef CALL_H_
#define CALL_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

class Call: public Quadruple {
public:
    Call(std::string procedureName);
    virtual ~Call() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getProcedureName() const;

private:
    void print(std::ostream& stream) const override;

    std::string procedureName;
};

} /* namespace code_generator */

#endif /* CALL_H_ */
