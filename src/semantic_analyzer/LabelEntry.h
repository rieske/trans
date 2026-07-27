#ifndef LABELENTRY_H_
#define LABELENTRY_H_

// LabelEntry lives in symbols. Compat re-export for SA; prefer symbols/ for new code.
#include "symbols/LabelEntry.h"

namespace semantic_analyzer {
using LabelEntry = symbols::LabelEntry;
}

#endif // LABELENTRY_H_
