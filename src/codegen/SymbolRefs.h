#ifndef CODEGEN_SYMBOL_REFS_H_
#define CODEGEN_SYMBOL_REFS_H_

#include <string>
#include <vector>

#include "codegen/Instruction.h"

namespace codegen {

// Use/def for frame packing / liveness (not print text).
struct SymbolRefs {
    std::vector<std::string> uses;
    std::vector<std::string> defs;
    bool isParam { false };
    bool isCall { false };
    // Non-empty only for AddressOf of an object (not FunctionAddress).
    std::string addressOfBase;

    void addUse(const std::string& name) {
        if (!name.empty()) {
            uses.push_back(name);
        }
    }
    void addDef(const std::string& name) {
        if (!name.empty()) {
            defs.push_back(name);
        }
    }
};

void collectSymbolRefs(const Instruction& instruction, SymbolRefs& refs);

} // namespace codegen

#endif // CODEGEN_SYMBOL_REFS_H_
