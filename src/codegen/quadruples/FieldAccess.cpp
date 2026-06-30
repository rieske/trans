#include "FieldAccess.h"
#include "codegen/AssemblyGenerator.h"
#include <iostream>

namespace codegen {

FieldAddress::FieldAddress(std::string base, int offsetBytes, std::string result, bool baseIsPointer) :
        base { std::move(base) },
        offsetBytes { offsetBytes },
        result { std::move(result) },
        baseIsPointer_ { baseIsPointer }
{
}

void FieldAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void FieldAddress::print(std::ostream& stream) const {
    stream << "\t" << result << " := &(" << base << (baseIsPointer_ ? "->" : ".") << offsetBytes << ")\n";
}

} // namespace codegen
