#ifndef CALL_H_
#define CALL_H_

#include <string>

#include "Quadruple.h"
#include "symbols/AddressPlan.h"

namespace codegen {

class Call: public Quadruple {
public:
    // Direct: procedureName is a function label.
    // Indirect: procedureName is the value holding the callee address.
    Call(std::string procedureName, symbols::CallPlan::Kind kind = symbols::CallPlan::Kind::Direct);
    virtual ~Call() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getProcedureName() const;
    symbols::CallPlan::Kind kind() const;
    bool isIndirect() const;

private:
    void print(std::ostream& stream) const override;

    std::string procedureName;
    symbols::CallPlan::Kind kind_ { symbols::CallPlan::Kind::Direct };
};

} // namespace codegen

#endif // CALL_H_
