#ifndef CALL_H_
#define CALL_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Call: public Quadruple {
public:
    // Direct: procedureName is a function label.
    // Indirect: procedureName is the value holding the callee address.
    Call(std::string procedureName, bool indirect = false);
    virtual ~Call() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getProcedureName() const;
    bool isIndirect() const;

private:
    void print(std::ostream& stream) const override;

    std::string procedureName;
    bool indirect_ { false };
};

} // namespace codegen

#endif // CALL_H_
