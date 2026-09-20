#ifndef CODEGEN_IRINLINE_H_
#define CODEGEN_IRINLINE_H_

#include "Instruction.h"

#include <string_view>
#include <vector>

namespace codegen {

struct InlineCaps {
    int maxCalleeInsts { 32 };
    int maxCallerInsts { 512 };
    int maxInlinesPerCaller { 64 };
    int maxTotalInlines { 256 };
};

bool isSetjmpFamily(std::string_view name);
bool calleeLooksUnsafe(const Procedure& callee, const IrStringTable& strings);
int nonLabelCount(const std::vector<Instruction>& body);

bool callIsEligible(const Instruction& call,
        const Procedure& caller,
        const Procedure& callee,
        const IrStringTable& strings,
        const InlineCaps& caps,
        int paramCount,
        int inlinesOnCaller,
        int inlinesInTu,
        bool calleeFinished);

bool formalCanShareActual(const Procedure& callee, int formalId, int actualId,
        const Procedure& caller);

struct InlineStats {
    int sitesConsidered { 0 };
    int sitesInlined { 0 };
    int refusedSize { 0 };
    int refusedRecursion { 0 };
    int refusedOther { 0 };
    std::vector<int> dirtyCallers;
};

// Does not splice. Pushes clones onto caller.frame.locals.
std::vector<Instruction> cloneCalleeBody(
        Procedure& caller,
        const Procedure& callee,
        const std::vector<int>& actuals,
        int retrieveResult,
        int memoryReturnDest,
        int siteId,
        IrStringTable& strings);

InlineStats inlineProcedures(IntermediateRepresentation& ir, InlineCaps caps = {});

} // namespace codegen

#endif
