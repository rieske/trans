#ifndef FUNCTIONENTRY_H_
#define FUNCTIONENTRY_H_

// FunctionEntry lives in symbols (host-aligned). This header re-exports for SA/AST.
#include "symbols/FunctionEntry.h"

namespace semantic_analyzer {
using FunctionEntry = symbols::FunctionEntry;
}

#endif // FUNCTIONENTRY_H_
