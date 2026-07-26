#include "IndexAddress.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

IndexAddress::IndexAddress(std::string base, std::string index, int elementSizeBytes, std::string result,
        symbols::AddressBaseMode baseMode) :
        base { std::move(base) },
        index { std::move(index) },
        elementSizeBytes { elementSizeBytes },
        result { std::move(result) },
        baseMode_ { baseMode }
{
}

void IndexAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void IndexAddress::print(std::ostream& stream) const {
    stream << "\t" << result << " := &" << base << "[" << index << "] stride=" << elementSizeBytes
            << (symbols::addressBaseUsesLea(baseMode_) ? " (array)\n" : " (ptr)\n");
}

} // namespace codegen
