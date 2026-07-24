#ifndef SEMANTIC_ANALYZER_SYMBOLS_H_
#define SEMANTIC_ANALYZER_SYMBOLS_H_

// Symbol table entries live in the symbols library (ast and SA both use them
// without creating an ast↔semantic_analyzer link cycle).

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/FunctionEntry.h"

namespace semantic_analyzer {

using symbols::ValueEntry;
using symbols::LabelEntry;
using symbols::FunctionEntry;

} // namespace semantic_analyzer

#endif // SEMANTIC_ANALYZER_SYMBOLS_H_
