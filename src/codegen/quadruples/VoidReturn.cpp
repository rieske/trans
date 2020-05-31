#include "VoidReturn.h"

#include "../AssemblyGenerator.h"

namespace codegen {

void VoidReturn::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool VoidReturn::transfersControl() const {
    return true;
}

void VoidReturn::print(std::ostream& stream) const {
    stream << "\tRETURN\n";
}

} /* namespace codegen */
