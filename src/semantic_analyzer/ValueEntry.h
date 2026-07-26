#ifndef VALUEENTRY_H_
#define VALUEENTRY_H_

// ValueEntry lives in symbols (host-aligned). This header re-exports for SA.
#include "symbols/ValueEntry.h"

namespace semantic_analyzer {
using ValueEntry = symbols::ValueEntry;
}

#endif
