#ifndef SYMBOLS_ANNOTATION_STORE_H_
#define SYMBOLS_ANNOTATION_STORE_H_

#include <optional>
#include <unordered_map>
#include <vector>

#include "AddressPlan.h"

// Side table for SA→CG facts (finish-for-git AnnotationStore subset).
// Expression Result values still live on ast::Expression for this stack slice;
// plans and field-init schedules live here so they are not AST syntax.

namespace symbols {

struct NodeAnnotations {
    std::optional<AddressPlan> addressPlan;
    std::optional<CallPlan> callPlan;
    std::vector<StructFieldInit> fieldInits;
};

class AnnotationStore {
public:
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
