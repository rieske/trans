#ifndef SYMBOLS_ANNOTATION_STORE_H_
#define SYMBOLS_ANNOTATION_STORE_H_

// Side-table decorations for analysis/codegen. Storage is NOT on AST node
// objects: keyed by node address. Bind a store for the duration of SA+codegen
// of one translation unit (see AnnotationStore::Bind).

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"

namespace symbols {

enum class ValueSlot {
    Result,
    Lvalue,
    PreOperation,
    Holder,
    Object,
    BaseResult,
    FieldAddress,
    CaseTemp,
};

enum class LabelSlot {
    Primary,     // generic single label (case, default, named label)
    Target,      // goto / break / continue jump target
    Falsy,
    Truthy,
    Exit,
    LoopEntry,
    LoopContinue,
    LoopExit,
};

struct FunctionFrame {
    std::unique_ptr<FunctionEntry> symbol;
    std::map<std::string, ValueEntry> locals;
    std::vector<ValueEntry> arguments;
    std::vector<std::string> parameterNames;
};

class AnnotationStore {
public:
    // RAII: bind this store as current for the thread.
    class Bind {
    public:
        explicit Bind(AnnotationStore& store);
        ~Bind();
        Bind(const Bind&) = delete;
        Bind& operator=(const Bind&) = delete;
    private:
        AnnotationStore* previous_ { nullptr };
    };

    static AnnotationStore& current();
    static bool hasCurrent();

    void setValue(const void* node, ValueSlot slot, ValueEntry value);
    ValueEntry* value(const void* node, ValueSlot slot);
    const ValueEntry* value(const void* node, ValueSlot slot) const;
    bool hasValue(const void* node, ValueSlot slot) const;

    void setLabel(const void* node, LabelSlot slot, LabelEntry label);
    LabelEntry* label(const void* node, LabelSlot slot);
    const LabelEntry* label(const void* node, LabelSlot slot) const;

    void setString(const void* node, std::string key, std::string value);
    const std::string* string(const void* node, std::string key) const;

    FunctionFrame& functionFrame(const void* node);
    const FunctionFrame* functionFrameIfAny(const void* node) const;

    void setFunctionSymbol(const void* node, FunctionEntry symbol);
    FunctionEntry* functionSymbol(const void* node);
    const FunctionEntry* functionSymbol(const void* node) const;

    void clear();

private:
    static thread_local AnnotationStore* current_;

    struct NodeValues {
        std::unordered_map<int, std::unique_ptr<ValueEntry>> slots;
    };
    struct NodeLabels {
        std::unordered_map<int, std::unique_ptr<LabelEntry>> slots;
    };
    struct NodeStrings {
        std::unordered_map<std::string, std::string> kv;
    };

    std::unordered_map<const void*, NodeValues> values_;
    std::unordered_map<const void*, NodeLabels> labels_;
    std::unordered_map<const void*, NodeStrings> strings_;
    std::unordered_map<const void*, FunctionFrame> functions_;
    std::unordered_map<const void*, std::unique_ptr<FunctionEntry>> callSymbols_;
};

} // namespace symbols

#endif // SYMBOLS_ANNOTATION_STORE_H_
