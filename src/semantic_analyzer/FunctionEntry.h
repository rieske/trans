#ifndef FUNCTIONENTRY_H_
#define FUNCTIONENTRY_H_

// FunctionEntry lives in symbols (host-aligned). This header re-exports compat re-export for SA (and residual AST includes until annotation migration); prefer symbols/ for new code.
#include "symbols/FunctionEntry.h"

namespace semantic_analyzer {
using FunctionEntry = symbols::FunctionEntry;
}

#endif // FUNCTIONENTRY_H_
