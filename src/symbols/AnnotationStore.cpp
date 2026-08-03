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

void AnnotationStore::setString(NodeRef node, StringSlot slot, std::string value) {
    this->node(node).strings[slot] = std::move(value);
}

const std::string* AnnotationStore::string(NodeRef node, StringSlot slot) const {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    auto it = n->strings.find(slot);
    if (it == n->strings.end()) {
        return nullptr;
    }
    return &it->second;
}

FunctionFrame& AnnotationStore::functionFrame(NodeRef node) {
    auto& n = this->node(node);
    if (!n.frame) {
        n.frame = std::make_unique<FunctionFrame>();
    }
    return *n.frame;
}

const FunctionFrame* AnnotationStore::functionFrameIfAny(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->frame) {
        return nullptr;
    }
    return n->frame.get();
}

void AnnotationStore::setFunctionSymbol(NodeRef node, FunctionEntry symbol) {
    this->node(node).callSymbol = std::make_unique<FunctionEntry>(std::move(symbol));
}

FunctionEntry* AnnotationStore::functionSymbol(NodeRef node) {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    return n->callSymbol.get();
}

const FunctionEntry* AnnotationStore::functionSymbol(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n) {
        return nullptr;
    }
    return n->callSymbol.get();
}

void AnnotationStore::addStructFieldInit(NodeRef node, StructFieldInit init) {
    this->node(node).fieldInits.push_back(std::move(init));
}

const std::vector<StructFieldInit>& AnnotationStore::structFieldInits(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n) {
        return kEmptyFieldInits;
    }
    return n->fieldInits;
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

void AnnotationStore::setPointerArithPlan(NodeRef node, PointerArithPlan plan) {
    this->node(node).pointerArithPlan = std::move(plan);
}

const PointerArithPlan* AnnotationStore::pointerArithPlan(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->pointerArithPlan) {
        return nullptr;
    }
    return &*n->pointerArithPlan;
}

void AnnotationStore::setCallPlan(NodeRef node, CallPlan plan) {
    auto& n = this->node(node);
    n.callPlan = std::move(plan);
    n.builtinPlan.reset();
}

const CallPlan* AnnotationStore::callPlan(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->callPlan) {
        return nullptr;
    }
    return &*n->callPlan;
}

void AnnotationStore::setBuiltinPlan(NodeRef node, BuiltinPlan plan) {
    auto& n = this->node(node);
    n.builtinPlan = std::move(plan);
    n.callPlan.reset();
}

const BuiltinPlan* AnnotationStore::builtinPlan(NodeRef node) const {
    auto* n = nodeIfAny(node);
    if (!n || !n->builtinPlan) {
        return nullptr;
    }
    return &*n->builtinPlan;
}


void AnnotationStore::clear() {
    nodes_.clear();
}

} // namespace symbols
