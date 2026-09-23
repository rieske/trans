#ifndef CODEGEN_LOOP_FACTS_H_
#define CODEGEN_LOOP_FACTS_H_

#include "Cfg.h"
#include "Instruction.h"
#include "Loops.h"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace codegen {

struct LoopSnap {
    std::vector<std::vector<std::size_t>> pred;
    std::vector<DomBits> dom;
    std::vector<NaturalLoop> loops;
};

struct UseDefIndex {
    std::unordered_map<int, Value*> values;
    std::unordered_map<int, int> defCount;
    std::unordered_map<int, std::unordered_set<std::size_t>> defBlocks;
    std::unordered_map<int, std::unordered_set<std::size_t>> useBlocks;
    std::unordered_set<int> addressTaken;
    std::vector<char> indirectWrite;
};

LoopSnap analyzeLoops(const Cfg& cfg);
UseDefIndex buildUseDefIndex(const Cfg& cfg, Procedure& procedure);
Value* findValue(const UseDefIndex& index, int id);
bool defInLoop(const UseDefIndex& index, const NaturalLoop& loop, int id);
bool loopHasIndirectWrite(const UseDefIndex& index, const NaturalLoop& loop);
void appendBeforeTerminator(BasicBlock& block, Instruction inst);
int insertPreheadersFor(Cfg& cfg, IrStringTable& strings, std::unordered_set<int> need);

} // namespace codegen

#endif // CODEGEN_LOOP_FACTS_H_
