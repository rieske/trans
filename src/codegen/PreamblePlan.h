#ifndef PREAMBLEPLAN_H_
#define PREAMBLEPLAN_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "GlobalVariable.h"

namespace codegen {

// Shared product policy for .data / extern / .text exports.
// Instruction sets only format this plan (NASM vs GAS).
struct PreambleDataItem {
    std::string name;
    bool exportGlobal { true }; // false for static file-scope
    // Exactly one of: string bytes token, or numeric operand list.
    std::optional<std::string> stringToken;
    std::vector<std::string> dataOperands;
    int widthBytes { 8 };
};

struct PreamblePlan {
    std::vector<std::string> externs; // sorted unique bare symbols
    std::vector<PreambleDataItem> data;
    std::vector<std::string> textGlobals; // non-static defined procedures
};

// Build plan from IR-collected maps/lists (same policy for Intel and AT&T).
PreamblePlan buildPreamblePlan(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions);

// Strip a single leading NASM `$` from a symbol if present.
std::string bareSymbol(std::string name);

} // namespace codegen

#endif
