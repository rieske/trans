#include "Assembly.h"

namespace codegen {

Assembly::Assembly(std::ostream* ostream):
    assembly { ostream }
{
}

Assembly& Assembly::operator<<(const std::string& instruction) {
    if (!instruction.empty()) {
        *assembly << "\t" << instruction;
    }
    return *this;
}

void Assembly::raw(std::string raw) {
    *assembly << raw;
}

void Assembly::label(std::string label) {
    *assembly << label << "\n";
}

}
