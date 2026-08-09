#include "InstructionSet.h"

#include <sstream>

namespace codegen {

std::string InstructionSet::preamblePrefix() const {
    return {};
}

std::string InstructionSet::globlDataLine(const std::string& name) const {
    return globl(name) + "\n";
}

std::string InstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions) const {
    std::stringstream out;
    out << preamblePrefix();
    for (const auto& name : externalFunctions) {
        out << externDirective(name) << "\n";
    }
    for (const auto& global : globalVariables) {
        if (global.emission == ObjectEmission::Reference) {
            out << externDirective(global.name) << "\n";
        }
    }
    out << dataSectionHeader();
    for (const auto& constant : constants) {
        out << constantLine(constant.first, constant.second);
    }
    for (const auto& global : globalVariables) {
        if (global.emission == ObjectEmission::Reference) {
            continue;
        }
        if (global.emission == ObjectEmission::DefineExternal) {
            out << globlDataLine(global.name);
        }
        out << dataObjectLines(global);
    }
    out << textSectionHeader();
    return out.str();
}

} // namespace codegen
