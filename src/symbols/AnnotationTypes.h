#ifndef SYMBOLS_ANNOTATIONTYPES_H_
#define SYMBOLS_ANNOTATIONTYPES_H_

// Shared annotation slot types used by AnnotationStore (SA→CG product).
// Keep this header free of FunctionEntry / frame types so store TUs stay light.

namespace symbols {

enum class ValueSlot {
    Result,
    // Production address temp for this expression (members, arrays, *).
    Lvalue,
    // Switch comparison temp.
    CaseTemp,
    // Postfix ++/-- value before the side effect.
    PreOperation,
    // Declarator object symbol (global/local storage).
    Holder,
    // Implicit float<->int convert destination (return / call args).
    Conversion,
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
