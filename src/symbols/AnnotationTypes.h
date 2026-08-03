#ifndef SYMBOLS_ANNOTATIONTYPES_H_
#define SYMBOLS_ANNOTATIONTYPES_H_

// Shared annotation payloads used by AnnotationStore (SA→CG product).
// Split from AnnotationStore.h so slot/frame types are include-able without
// the full store implementation surface.

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"
#include "types/Type.h"

namespace symbols {

enum class ValueSlot {
    Result,
    Lvalue, // sole "address of this expression" temp (members, arrays, …)
    Holder,
    Object,
    CaseTemp,
    Conversion, // implicit convert dest (float/int width, or bool 0/1)
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
    ConstantLabel,
};

// SA-owned source + address temps so frame layout stays stable vs CL homes.
struct FieldInitTemps {
    std::unique_ptr<ValueEntry> source;
    std::unique_ptr<ValueEntry> address;
};

// One local-aggregate store row (brace/designated/string init). Closed arms;
// CG switches with std::get_if / visit.
struct FieldZeroSpan {
    int offsetBytes { 0 };
    int zeroSpanBytes { 0 };
    FieldInitTemps temps;
};

struct FieldConstant {
    int offsetBytes { 0 };
    type::Type storeType { type::voidType() };
    std::string constantValue;
    std::optional<type::BitField> bitField;
    FieldInitTemps temps;
};

struct FieldAddressOf {
    int offsetBytes { 0 };
    type::Type storeType { type::voidType() };
    std::string addressOfOperand;
    FieldInitTemps temps;
};

struct FieldValue {
    int offsetBytes { 0 };
    type::Type storeType { type::voidType() };
    std::optional<type::BitField> bitField;
    FieldInitTemps temps;
};

struct FieldStringBytes {
    int offsetBytes { 0 };
    int sizeBytes { 0 };
    std::vector<unsigned char> bytes;
    FieldInitTemps temps;
};

using FieldInit = std::variant<
        FieldZeroSpan,
        FieldConstant,
        FieldAddressOf,
        FieldValue,
        FieldStringBytes>;

inline FieldInit fieldZeroSpan(int offsetBytes, int spanBytes, FieldInitTemps temps) {
    return FieldZeroSpan { offsetBytes, spanBytes, std::move(temps) };
}

inline FieldInit fieldConstant(int offsetBytes, type::Type storeType, std::string value,
        FieldInitTemps temps, std::optional<type::BitField> bits = {}) {
    return FieldConstant { offsetBytes, std::move(storeType), std::move(value), bits, std::move(temps) };
}

inline FieldInit fieldAddressOf(int offsetBytes, type::Type storeType, std::string operand,
        FieldInitTemps temps) {
    return FieldAddressOf { offsetBytes, std::move(storeType), std::move(operand), std::move(temps) };
}

inline FieldInit fieldValue(int offsetBytes, type::Type storeType, FieldInitTemps temps,
        std::optional<type::BitField> bits = {}) {
    return FieldValue { offsetBytes, std::move(storeType), bits, std::move(temps) };
}

inline FieldInit fieldStringBytes(int offsetBytes, int sizeBytes, std::vector<unsigned char> bytes,
        FieldInitTemps temps) {
    return FieldStringBytes { offsetBytes, sizeBytes, std::move(bytes), std::move(temps) };
}

// Per-function frame: written by SA, read by CG StartProcedure.
struct FunctionFrame {
    std::unique_ptr<FunctionEntry> symbol;
    std::map<std::string, ValueEntry> locals;
    std::vector<ValueEntry> arguments;
};

} // namespace symbols

#endif // SYMBOLS_ANNOTATIONTYPES_H_
