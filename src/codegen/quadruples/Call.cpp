#include "Call.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Call::Call(std::string procedureName, symbols::CallPlan::Kind kind) :
        procedureName { std::move(procedureName) },
        kind_ { kind }
{
}

void Call::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Call::getProcedureName() const {
    return procedureName;
}

symbols::CallPlan::Kind Call::kind() const {
    return kind_;
}

bool Call::isIndirect() const {
    return kind_ == symbols::CallPlan::Kind::Indirect;
}

void Call::print(std::ostream& stream) const {
    if (isIndirect()) {
        stream << "\tCALL *" << getProcedureName() << "\n";
    } else {
        stream << "\tCALL " << getProcedureName() << "\n";
    }
}

} // namespace codegen
