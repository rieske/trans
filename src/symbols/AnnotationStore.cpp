#include "AnnotationStore.h"

#include <stdexcept>

namespace symbols {

thread_local AnnotationStore* AnnotationStore::current_ = nullptr;

AnnotationStore::Bind::Bind(AnnotationStore& store)
        : previous_ { AnnotationStore::current_ } {
    AnnotationStore::current_ = &store;
}

AnnotationStore::Bind::~Bind() {
    AnnotationStore::current_ = previous_;
}

AnnotationStore& AnnotationStore::current() {
    if (!current_) {
        throw std::runtime_error { "AnnotationStore not bound for this thread" };
    }
    return *current_;
}

bool AnnotationStore::hasCurrent() {
    return current_ != nullptr;
}

void AnnotationStore::setValue(const void* node, ValueSlot slot, ValueEntry value) {
    values_[node].slots[static_cast<int>(slot)] =
            std::make_unique<ValueEntry>(std::move(value));
}

ValueEntry* AnnotationStore::value(const void* node, ValueSlot slot) {
    auto it = values_.find(node);
    if (it == values_.end()) {
        return nullptr;
    }
    auto sit = it->second.slots.find(static_cast<int>(slot));
    if (sit == it->second.slots.end()) {
        return nullptr;
    }
    return sit->second.get();
}

const ValueEntry* AnnotationStore::value(const void* node, ValueSlot slot) const {
    return const_cast<AnnotationStore*>(this)->value(node, slot);
}

bool AnnotationStore::hasValue(const void* node, ValueSlot slot) const {
    return value(node, slot) != nullptr;
}

void AnnotationStore::setLabel(const void* node, LabelSlot slot, LabelEntry label) {
    labels_[node].slots[static_cast<int>(slot)] =
            std::make_unique<LabelEntry>(std::move(label));
}

LabelEntry* AnnotationStore::label(const void* node, LabelSlot slot) {
    auto it = labels_.find(node);
    if (it == labels_.end()) {
        return nullptr;
    }
    auto sit = it->second.slots.find(static_cast<int>(slot));
    if (sit == it->second.slots.end()) {
        return nullptr;
    }
    return sit->second.get();
}

const LabelEntry* AnnotationStore::label(const void* node, LabelSlot slot) const {
    return const_cast<AnnotationStore*>(this)->label(node, slot);
}

void AnnotationStore::setString(const void* node, std::string key, std::string value) {
    strings_[node].kv[std::move(key)] = std::move(value);
}

const std::string* AnnotationStore::string(const void* node, std::string key) const {
    auto it = strings_.find(node);
    if (it == strings_.end()) {
        return nullptr;
    }
    auto sit = it->second.kv.find(key);
    if (sit == it->second.kv.end()) {
        return nullptr;
    }
    return &sit->second;
}

FunctionFrame& AnnotationStore::functionFrame(const void* node) {
    return functions_[node];
}

const FunctionFrame* AnnotationStore::functionFrameIfAny(const void* node) const {
    auto it = functions_.find(node);
    if (it == functions_.end()) {
        return nullptr;
    }
    return &it->second;
}

void AnnotationStore::setFunctionSymbol(const void* node, FunctionEntry symbol) {
    callSymbols_[node] = std::make_unique<FunctionEntry>(std::move(symbol));
}

FunctionEntry* AnnotationStore::functionSymbol(const void* node) {
    auto it = callSymbols_.find(node);
    if (it == callSymbols_.end()) {
        return nullptr;
    }
    return it->second.get();
}

const FunctionEntry* AnnotationStore::functionSymbol(const void* node) const {
    return const_cast<AnnotationStore*>(this)->functionSymbol(node);
}

void AnnotationStore::clear() {
    values_.clear();
    labels_.clear();
    strings_.clear();
    functions_.clear();
    callSymbols_.clear();
}

} // namespace symbols
