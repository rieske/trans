#include "PointerDiff.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

PointerDiff::PointerDiff(std::string left, std::string right, int elementSizeBytes, std::string result) :
        left { std::move(left) },
        right { std::move(right) },
        elementSizeBytes { elementSizeBytes },
        result { std::move(result) } {
}

void PointerDiff::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void PointerDiff::print(std::ostream& stream) const {
    stream << "\t" << result << " := (" << left << " - " << right << ")";
    if (elementSizeBytes != 1) {
        stream << " /" << elementSizeBytes;
    }
    stream << " (ptrdiff)\n";
}

} // namespace codegen
