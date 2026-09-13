#ifndef CODEGEN_IRINLINE_H_
#define CODEGEN_IRINLINE_H_

#include "Instruction.h"

#include <string_view>

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

} // namespace codegen

#endif
