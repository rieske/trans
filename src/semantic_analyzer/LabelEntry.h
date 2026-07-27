#ifndef LABELENTRY_H_
#define LABELENTRY_H_

// LabelEntry lives in symbols (host-aligned). This header re-exports for SA/AST.
#include "symbols/LabelEntry.h"

namespace semantic_analyzer {
using LabelEntry = symbols::LabelEntry;
}

#endif // LABELENTRY_H_
