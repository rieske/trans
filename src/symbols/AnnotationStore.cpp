#include "AnnotationStore.h"

namespace symbols {

const std::vector<StructFieldInit> AnnotationStore::kEmptyFieldInits {};

NodeAnnotations& AnnotationStore::node(NodeRef key) {
    return nodes_[key];
}

const NodeAnnotations* AnnotationStore::nodeIfAny(NodeRef key) const {
    auto it = nodes_.find(key);
    if (it == nodes_.end()) {
        return nullptr;
    }
    return &it->second;
}

void AnnotationStore::setAddressPlan(NodeRef node, AddressPlan plan) {
    this->node(node).addressPlan = std::move(plan);
}

const AddressPlan* AnnotationStore::addressPlan(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->addressPlan) {
        return nullptr;
    }
    return &*n->addressPlan;
}

void AnnotationStore::setCallPlan(NodeRef node, CallPlan plan) {
    this->node(node).callPlan = std::move(plan);
}

const CallPlan* AnnotationStore::callPlan(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->callPlan) {
        return nullptr;
    }
    return &*n->callPlan;
}

void AnnotationStore::addStructFieldInit(NodeRef node, StructFieldInit init) {
    this->node(node).fieldInits.push_back(std::move(init));
}

void AnnotationStore::setStructFieldInits(NodeRef node, std::vector<StructFieldInit> inits) {
    this->node(node).fieldInits = std::move(inits);
}

const std::vector<StructFieldInit>& AnnotationStore::structFieldInits(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n) {
        return kEmptyFieldInits;
    }
    return n->fieldInits;
}

void AnnotationStore::clear() {
    nodes_.clear();
}

} // namespace symbols
