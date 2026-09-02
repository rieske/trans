#include "AnnotationStore.h"

#include <cassert>

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

void AnnotationStore::setValue(NodeRef node, ValueSlot slot, ValueEntry value) {
    this->node(node).values[slot] = std::make_unique<ValueEntry>(std::move(value));
}

ValueEntry* AnnotationStore::value(NodeRef node, ValueSlot slot) {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    auto it = n->values.find(slot);
    if (it == n->values.end()) {
        return nullptr;
    }
    return it->second.get();
}

const ValueEntry* AnnotationStore::value(NodeRef node, ValueSlot slot) const {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    auto it = n->values.find(slot);
    if (it == n->values.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool AnnotationStore::hasValue(NodeRef node, ValueSlot slot) const {
    return value(node, slot) != nullptr;
}

ValueEntry* AnnotationStore::result(NodeRef node) {
    auto* r = value(node, ValueSlot::Result);
    assert(r && "Result annotation required but missing");
    return r;
}

const ValueEntry* AnnotationStore::result(NodeRef node) const {
    auto* r = value(node, ValueSlot::Result);
    assert(r && "Result annotation required but missing");
    return r;
}

void AnnotationStore::setLabel(NodeRef node, LabelSlot slot, LabelEntry label) {
    this->node(node).labels[slot] = std::make_unique<LabelEntry>(std::move(label));
}

LabelEntry* AnnotationStore::label(NodeRef node, LabelSlot slot) {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    auto it = n->labels.find(slot);
    if (it == n->labels.end()) {
        return nullptr;
    }
    return it->second.get();
}

const LabelEntry* AnnotationStore::label(NodeRef node, LabelSlot slot) const {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    auto it = n->labels.find(slot);
    if (it == n->labels.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool AnnotationStore::hasLabel(NodeRef node, LabelSlot slot) const {
    return label(node, slot) != nullptr;
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

void AnnotationStore::setFunctionFrame(NodeRef node, FunctionFrame frame) {
    this->node(node).functionFrame = std::move(frame);
}

const FunctionFrame* AnnotationStore::functionFrame(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->functionFrame) {
        return nullptr;
    }
    return &*n->functionFrame;
}

void AnnotationStore::setRodataLabel(NodeRef node, std::string label) {
    this->node(node).rodataLabel = std::move(label);
}

const std::string* AnnotationStore::rodataLabel(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->rodataLabel) {
        return nullptr;
    }
    return &*n->rodataLabel;
}

void AnnotationStore::setSizeofValue(NodeRef node, int bytes) {
    this->node(node).sizeofValue = bytes;
}

const int* AnnotationStore::sizeofValue(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->sizeofValue) {
        return nullptr;
    }
    return &*n->sizeofValue;
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
