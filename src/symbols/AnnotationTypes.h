#ifndef SYMBOLS_ANNOTATIONTYPES_H_
#define SYMBOLS_ANNOTATIONTYPES_H_

// Shared annotation payloads used by AnnotationStore (SA→CG product).

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"

namespace symbols {

enum class ValueSlot {
    Result,
    // Sole "address of this expression" temp (members, arrays, …) - migration target.
    Lvalue,
    PreOperation,
    Holder,
    Object,
    CaseTemp,
};

enum class LabelSlot {
    Primary,
    Target,
    Falsy,
    Truthy,
    Exit,
    LoopEntry,
    LoopContinue,
    LoopExit,
};

enum class StringSlot {
    ArrayDecaySource,
    ConversionTarget,
};

// Per-function frame: written by SA, read by CG StartProcedure (Phase 0.5+).
struct FunctionFrame {
    std::unique_ptr<FunctionEntry> symbol;
    std::map<std::string, ValueEntry> locals;
    std::vector<ValueEntry> arguments;
    std::vector<std::string> parameterNames;
};

} // namespace symbols

#endif // SYMBOLS_ANNOTATIONTYPES_H_
