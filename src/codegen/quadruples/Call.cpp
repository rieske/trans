#include "Call.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Call::Call(std::string procedureName, bool indirect) :
        procedureName { std::move(procedureName) },
        indirect_ { indirect }
{
}

void Call::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Call::getProcedureName() const {
    return procedureName;
}

bool Call::isIndirect() const {
    return indirect_;
}

void Call::print(std::ostream& stream) const {
    if (indirect_) {
        stream << "\tCALL *" << getProcedureName() << "\n";
    } else {
        stream << "\tCALL " << getProcedureName() << "\n";
    }
}

} // namespace codegen
