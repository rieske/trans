#ifndef SYMBOLS_ANNOTATIONTYPES_H_
#define SYMBOLS_ANNOTATIONTYPES_H_

// Shared annotation slot types used by AnnotationStore (SA→CG product).
// Keep this header free of FunctionEntry / frame types so store TUs stay light.
// FunctionFrame and extra slots land with the migration that uses them.

namespace symbols {

enum class ValueSlot {
    Result,
    // Production address temp for this expression (members, arrays, *).
    Lvalue,
    // Switch comparison temp (was on SwitchStatement).
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

} // namespace symbols

#endif // SYMBOLS_ANNOTATIONTYPES_H_
