#ifndef SYMBOLS_ANNOTATIONTYPES_H_
#define SYMBOLS_ANNOTATIONTYPES_H_

// Shared annotation payloads used by AnnotationStore (SA→CG product).
// Split from AnnotationStore.h so slot/frame types are include-able without
// the full store implementation surface.

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"

namespace symbols {

enum class ValueSlot {
    Result,
    Lvalue, // sole "address of this expression" temp (members, arrays, …)
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

// One store into an aggregate init plan. Exactly one of:
//   - zeroSpanBytes > 0: zero [offsetBytes, offsetBytes+zeroSpanBytes)
//   - constantValue / addressOfOperand / source value: store that into *address
struct StructFieldInit {
    int offsetBytes { 0 };
    int zeroSpanBytes { 0 };
    std::optional<std::string> constantValue;
    std::unique_ptr<ValueEntry> source;
    std::unique_ptr<ValueEntry> address;
    std::optional<std::string> addressOfOperand;
};

// Per-function frame: written by SA, read by CG StartProcedure.
struct FunctionFrame {
    std::unique_ptr<FunctionEntry> symbol;
    std::map<std::string, ValueEntry> locals;
    std::vector<ValueEntry> arguments;
    std::vector<std::string> parameterNames;
};

} // namespace symbols

#endif // SYMBOLS_ANNOTATIONTYPES_H_
