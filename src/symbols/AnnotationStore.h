#ifndef SYMBOLS_ANNOTATION_STORE_H_
#define SYMBOLS_ANNOTATION_STORE_H_

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "AddressPlan.h"
#include "AnnotationTypes.h"
#include "ValueEntry.h"

// Side table for SA→CG facts.
// Result and plans live here. Lvalue/labels: API ready; production migration in Phase 0.5.

namespace symbols {

struct NodeAnnotations {
    std::unordered_map<ValueSlot, std::unique_ptr<ValueEntry>> values;
    std::unordered_map<LabelSlot, std::unique_ptr<LabelEntry>> labels;
    std::optional<AddressPlan> addressPlan;
    std::optional<CallPlan> callPlan;
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

    // Lvalue API ready; SA still writes node fields until Phase 0.5 migration.
    void setLvalue(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::Lvalue, std::move(value));
    }
    ValueEntry* lvalue(NodeRef node) { return value(node, ValueSlot::Lvalue); }
    const ValueEntry* lvalue(NodeRef node) const { return value(node, ValueSlot::Lvalue); }

    void setLabel(NodeRef node, LabelSlot slot, LabelEntry label);
    LabelEntry* label(NodeRef node, LabelSlot slot);
    const LabelEntry* label(NodeRef node, LabelSlot slot) const;
    bool hasLabel(NodeRef node, LabelSlot slot) const;

    void setAddressPlan(NodeRef node, AddressPlan plan);
    const AddressPlan* addressPlan(NodeRef node) const;

    void setCallPlan(NodeRef node, CallPlan plan);
    const CallPlan* callPlan(NodeRef node) const;

    void addStructFieldInit(NodeRef node, StructFieldInit init);
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
