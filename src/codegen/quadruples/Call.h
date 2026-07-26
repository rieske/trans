#ifndef CALL_H_
#define CALL_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// Mirrors symbols::CallPlan arms without depending on the full variant in IR.
enum class CallKind { Direct, Indirect };

class Call: public Quadruple {
public:
    // Direct: procedureName is a function label.
    // Indirect: procedureName is the value holding the callee address.
    Call(std::string procedureName, CallKind kind = CallKind::Direct);
    virtual ~Call() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getProcedureName() const;
    CallKind kind() const;
    bool isIndirect() const;

private:
    void print(std::ostream& stream) const override;

    std::string procedureName;
    CallKind kind_ { CallKind::Direct };
};

} // namespace codegen

#endif // CALL_H_
