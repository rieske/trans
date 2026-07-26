#include "FieldAddress.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

FieldAddress::FieldAddress(std::string base, int offsetBytes, std::string result,
        symbols::AddressBaseMode baseMode) :
        base { std::move(base) },
        offsetBytes { offsetBytes },
        result { std::move(result) },
        baseMode_ { baseMode }
{
}

void FieldAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void FieldAddress::print(std::ostream& stream) const {
    const char* op = symbols::addressBaseIsPointerValue(baseMode_) ? "->" : ".";
    stream << "\t" << result << " := &(" << base << op << offsetBytes << ")\n";
}

} // namespace codegen
