#ifndef SYMBOLS_ANNOTATION_STORE_H_
#define SYMBOLS_ANNOTATION_STORE_H_

// Side-table decorations for one TU. Keyed by NodeRef (not on AST objects).
//
// Packaging:
//   AnnotationTypes.h — ValueSlot/LabelSlot/StringSlot, StructFieldInit, FunctionFrame
//   AddressPlan / CallPlan / BuiltinPlan / PointerArithPlan — SA→CG plans
//   This store — map ownership + accessors
//
// All decorations here are SA→CG product (codegen reads them). Parse-time bridges
// (va_arg types) live on the AST / SA visitor, not here.

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AddressPlan.h"
#include "AnnotationTypes.h"
#include "BuiltinPlan.h"
#include "CallPlan.h"
#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "NodeRef.h"
#include "PointerArithPlan.h"
#include "ValueEntry.h"

namespace symbols {

// One node record: value/label/string slots, frame, call metadata, plans.
struct NodeAnnotations {
    std::map<ValueSlot, std::unique_ptr<ValueEntry>> values;
    std::map<LabelSlot, std::unique_ptr<LabelEntry>> labels;
    std::map<StringSlot, std::string> strings;
    std::unique_ptr<FunctionFrame> frame;
    std::unique_ptr<FunctionEntry> callSymbol;
    std::vector<StructFieldInit> fieldInits;
    std::optional<AddressPlan> addressPlan;
    std::optional<PointerArithPlan> pointerArithPlan;
    std::optional<CallPlan> callPlan;
    std::optional<BuiltinPlan> builtinPlan;
};

class AnnotationStore {
public:
    void setValue(NodeRef node, ValueSlot slot, ValueEntry value);
    ValueEntry* value(NodeRef node, ValueSlot slot);
    const ValueEntry* value(NodeRef node, ValueSlot slot) const;
    bool hasValue(NodeRef node, ValueSlot slot) const;

    void setResult(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::Result, std::move(value));
    }
    ValueEntry* result(NodeRef node);
    const ValueEntry* result(NodeRef node) const;
    bool hasResult(NodeRef node) const {
        return hasValue(node, ValueSlot::Result);
    }

    void setLabel(NodeRef node, LabelSlot slot, LabelEntry label);
    LabelEntry* label(NodeRef node, LabelSlot slot);
    const LabelEntry* label(NodeRef node, LabelSlot slot) const;

    void setString(NodeRef node, StringSlot slot, std::string value);
    const std::string* string(NodeRef node, StringSlot slot) const;

    FunctionFrame& functionFrame(NodeRef node);
    const FunctionFrame* functionFrameIfAny(NodeRef node) const;

    void setFunctionSymbol(NodeRef node, FunctionEntry symbol);
    FunctionEntry* functionSymbol(NodeRef node);
    const FunctionEntry* functionSymbol(NodeRef node) const;

    void addStructFieldInit(NodeRef node, StructFieldInit init);
    const std::vector<StructFieldInit>& structFieldInits(NodeRef node) const;

    void setAddressPlan(NodeRef node, AddressPlan plan);
    const AddressPlan* addressPlan(NodeRef node) const;

    void setPointerArithPlan(NodeRef node, PointerArithPlan plan);
    const PointerArithPlan* pointerArithPlan(NodeRef node) const;

    void setCallPlan(NodeRef node, CallPlan plan);
    const CallPlan* callPlan(NodeRef node) const;

    void setBuiltinPlan(NodeRef node, BuiltinPlan plan);
    const BuiltinPlan* builtinPlan(NodeRef node) const;

    void clear();

private:
    NodeAnnotations& node(NodeRef key);
    const NodeAnnotations* nodeIfAny(NodeRef key) const;

    std::unordered_map<NodeRef, NodeAnnotations, NodeRefHash> nodes_;
    static const std::vector<StructFieldInit> kEmptyFieldInits;
};

} // namespace symbols

#endif // SYMBOLS_ANNOTATION_STORE_H_
