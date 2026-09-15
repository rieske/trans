#ifndef CODEGEN_SYMBOL_REFS_H_
#define CODEGEN_SYMBOL_REFS_H_

#include "codegen/Instruction.h"
#include "codegen/InternalError.h"

namespace codegen {

struct SymbolIdList {
    static constexpr int kCapacity = 2;
    int ids[kCapacity] {};
    int count { 0 };

    void push(int id) {
        if (count >= kCapacity) {
            internalError("SymbolRefs overflow");
        }
        ids[count++] = id;
    }

    const int* begin() const { return ids; }
    const int* end() const { return ids + count; }
    std::size_t size() const { return static_cast<std::size_t>(count); }
    bool empty() const { return count == 0; }
    int operator[](std::size_t i) const { return ids[i]; }
};

struct SymbolRefs {
    SymbolIdList uses;
    SymbolIdList defs;
    bool isParam { false };
    bool isCall { false };
    int addressOfBase { kNoSymbol };

    void addUse(int id) {
        if (id != kNoSymbol) {
            uses.push(id);
        }
    }
    void addDef(int id) {
        if (id != kNoSymbol) {
            defs.push(id);
        }
    }
};

void collectSymbolRefs(const Instruction& instruction, SymbolRefs& refs);

} // namespace codegen

#endif
