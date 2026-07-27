#include "PointerOffset.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

PointerOffset::PointerOffset(std::string base, std::string index, int elementSizeBytes, std::string result,
        bool subtract) :
        base { std::move(base) },
        index { std::move(index) },
        elementSizeBytes { elementSizeBytes },
        result { std::move(result) },
        subtract { subtract } {
}

void PointerOffset::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void PointerOffset::print(std::ostream& stream) const {
    stream << "\t" << result << " := " << base << (subtract ? " - " : " + ") << index;
    if (elementSizeBytes != 1) {
        stream << "*" << elementSizeBytes;
    }
    stream << " (ptr)\n";
}

} // namespace codegen
