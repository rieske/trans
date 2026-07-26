#ifndef SYMBOLS_ANNOTATION_STORE_H_
#define SYMBOLS_ANNOTATION_STORE_H_

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "AddressPlan.h"
#include "ValueEntry.h"

// Side table for SA→CG facts (finish-for-git AnnotationStore subset).
// Value annotations (Result / Lvalue) and plans live here — not on AST syntax nodes.

namespace symbols {

enum class ValueSlot {
    Result,
    // Sole address temp for this expression when stored on the side table
    // (array/member lvalues still use node fields until a follow-up migrates them).
    Lvalue,
};

struct NodeAnnotations {
    std::unordered_map<ValueSlot, std::unique_ptr<ValueEntry>> values;
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
    ValueEntry* result(NodeRef node);
    const ValueEntry* result(NodeRef node) const;
    bool hasResult(NodeRef node) const { return hasValue(node, ValueSlot::Result); }

    void setLvalue(NodeRef node, ValueEntry value) {
        setValue(node, ValueSlot::Lvalue, std::move(value));
    }
    ValueEntry* lvalue(NodeRef node) { return value(node, ValueSlot::Lvalue); }
    const ValueEntry* lvalue(NodeRef node) const { return value(node, ValueSlot::Lvalue); }

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
