#include "IndexAddress.h"
#include "codegen/AssemblyGenerator.h"
#include <iostream>

namespace codegen {

IndexAddress::IndexAddress(std::string base, std::string index, int elementSizeBytes, std::string result, bool baseIsArray) :
        base { std::move(base) },
        index { std::move(index) },
        elementSizeBytes { elementSizeBytes },
        result { std::move(result) },
        baseIsArray_ { baseIsArray }
{
}

void IndexAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void IndexAddress::print(std::ostream& stream) const {
    stream << "\t" << result << " := &" << base << "[" << index << "] stride=" << elementSizeBytes
            << (baseIsArray_ ? " (array)\n" : " (ptr)\n");
}


void IndexAddress::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(base);
    refs.addUse(index);
    refs.addDef(result);
}

} // namespace codegen
