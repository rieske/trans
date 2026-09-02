#ifndef SYMBOLS_ANNOTATION_STORE_H_
#define SYMBOLS_ANNOTATION_STORE_H_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AddressPlan.h"
#include "AnnotationTypes.h"
#include "FunctionFrame.h"
#include "LabelEntry.h"
#include "ValueEntry.h"

// Side table for SA→CG facts.
// Result and plans live here. Lvalue and control-flow labels are store-backed (production).

namespace symbols {

struct NodeAnnotations {
    std::unordered_map<ValueSlot, std::unique_ptr<ValueEntry>> values;
    std::unordered_map<LabelSlot, std::unique_ptr<LabelEntry>> labels;
    std::optional<AddressPlan> addressPlan;
    std::optional<CallPlan> callPlan;
    std::optional<FunctionFrame> functionFrame;
    std::optional<std::string> rodataLabel;
    std::optional<int> sizeofValue;
    std::vector<StructFieldInit> fieldInits;
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
    // Required Result after successful SA (asserts if missing). Prefer hasResult + value() for probes.
    ValueEntry* result(NodeRef node);
    const ValueEntry* result(NodeRef node) const;
    bool hasResult(NodeRef node) const { return hasValue(node, ValueSlot::Result); }

    // Lvalue address temps (arrays, members, *).
    void setLvalue(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::Lvalue, std::move(value));
    }
    ValueEntry* lvalue(NodeRef node) { return value(node, ValueSlot::Lvalue); }
    const ValueEntry* lvalue(NodeRef node) const { return value(node, ValueSlot::Lvalue); }

    void setCaseTemp(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::CaseTemp, std::move(value));
    }
    ValueEntry* caseTemp(NodeRef node) { return value(node, ValueSlot::CaseTemp); }
    const ValueEntry* caseTemp(NodeRef node) const { return value(node, ValueSlot::CaseTemp); }

    void setPreOperation(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::PreOperation, std::move(value));
    }
    ValueEntry* preOperation(NodeRef node) { return value(node, ValueSlot::PreOperation); }
    const ValueEntry* preOperation(NodeRef node) const { return value(node, ValueSlot::PreOperation); }

    void setHolder(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::Holder, std::move(value));
    }
    ValueEntry* holder(NodeRef node) { return value(node, ValueSlot::Holder); }
    const ValueEntry* holder(NodeRef node) const { return value(node, ValueSlot::Holder); }

    void setConversion(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::Conversion, std::move(value));
    }
    ValueEntry* conversion(NodeRef node) { return value(node, ValueSlot::Conversion); }
    const ValueEntry* conversion(NodeRef node) const { return value(node, ValueSlot::Conversion); }

    void setLabel(NodeRef node, LabelSlot slot, LabelEntry label);
    LabelEntry* label(NodeRef node, LabelSlot slot);
    const LabelEntry* label(NodeRef node, LabelSlot slot) const;

    void setAddressPlan(NodeRef node, AddressPlan plan);
    const AddressPlan* addressPlan(NodeRef node) const;

    void setCallPlan(NodeRef node, CallPlan plan);
    const CallPlan* callPlan(NodeRef node) const;

    void setFunctionFrame(NodeRef node, FunctionFrame frame);
    const FunctionFrame* functionFrame(NodeRef node) const;

    void setRodataLabel(NodeRef node, std::string label);
    const std::string* rodataLabel(NodeRef node) const;

    void setSizeofValue(NodeRef node, int bytes);
    const int* sizeofValue(NodeRef node) const;

    void setStructFieldInits(NodeRef node, std::vector<StructFieldInit> inits);
    const std::vector<StructFieldInit>& structFieldInits(NodeRef node) const;

    void clear();

private:
    NodeAnnotations& node(NodeRef key);
    const NodeAnnotations* nodeIfAny(NodeRef key) const;

    std::unordered_map<NodeRef, NodeAnnotations, NodeRefHash> nodes_;
    static const std::vector<StructFieldInit> kEmptyFieldInits;
};

} // namespace symbols

#endif // SYMBOLS_ANNOTATION_STORE_H_
