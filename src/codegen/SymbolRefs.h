#ifndef CODEGEN_SYMBOL_REFS_H_
#define CODEGEN_SYMBOL_REFS_H_

#include <vector>

#include "codegen/Instruction.h"

namespace codegen {

struct SymbolRefs {
    std::vector<int> uses;
    std::vector<int> defs;
    bool isParam { false };
    bool isCall { false };
    int addressOfBase { kNoSymbol };

    void addUse(int id) {
        if (id != kNoSymbol) {
            uses.push_back(id);
        }
    }
    void addDef(int id) {
        if (id != kNoSymbol) {
            defs.push_back(id);
        }
    }
};

void collectSymbolRefs(const Instruction& instruction, SymbolRefs& refs);

} // namespace codegen

#endif
