#ifndef AGGREGATE_INIT_SINK_H_
#define AGGREGATE_INIT_SINK_H_

#include "types/ObjectAbi.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"
#include "translation_unit/Context.h"

#include <functional>
#include <string>

namespace ast {
class Expression;
}

namespace semantic_analyzer {

inline constexpr const char* kIncompleteArrayInitMsg =
        "array brace initializers for incomplete arrays are not implemented";

inline bool isCharArrayType(const type::Type& t) {
    return t.isArray() && t.getArraySize() > 0 && t.getElementType().isPrimitive()
            && t.getElementType().getSize() == 1;
}

// Thin host for sinks: annotations + diagnostics only (no visitor).
struct AggregateInitHost {
    symbols::AnnotationStore& annotations;
    std::function<void(std::string message, const translation_unit::Context& context)> error;
};

// Policy sink for one placement pass over an aggregate initializer.
struct AggregateInitSink {
    virtual ~AggregateInitSink() = default;
    // Leaf scalar store (not char[] string packing).
    virtual void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) = 0;
    // char[N] from a string literal. Return true if handled.
    virtual bool placeStringArray(int offsetBytes, const type::Type& arrayType,
            ast::Expression* value) = 0;
    // Whole record/array copy from a live expression result (e.g. .ref = *ref).
    // Return true if handled so the walk does not peel into the first subobject.
    virtual bool placeAggregateCopy(int /*offsetBytes*/, const type::Type& /*storeType*/,
            ast::Expression* /*value*/) {
        return false;
    }
    virtual void onUnwritten(int offsetBytes, const type::Type& t) = 0;
    virtual void error(const std::string& message) = 0;
    virtual bool ok() const = 0;
};

// Cover [offsetBytes, offsetBytes+size) with machine-word / int / byte stores.
// Used for brace zero-init of aggregates so padding between members is zeroed
// (C 6.7.9: remainder of an aggregate is initialized as if static storage;
// git ref-filter relies on memcmp of struct object_info empty = { 0 }).
template<typename Leaf>
void fillStorageUnitsBySize(int offsetBytes, int size, Leaf&& leaf) {
    int off = 0;
    while (off + type::object_abi::MACHINE_WORD_SIZE <= size) {
        leaf(offsetBytes + off, type::signedLong());
        off += type::object_abi::MACHINE_WORD_SIZE;
    }
    if (off + 4 <= size) {
        leaf(offsetBytes + off, type::signedInteger());
        off += 4;
    }
    while (off < size) {
        leaf(offsetBytes + off, type::signedCharacter());
        ++off;
    }
}

// Shared layout walk: invoke leaf(offset, storeType) for each scalar storage unit.
// Aggregates (struct/union/array) are filled by total size so inter-member and
// trailing padding are included. Scalars are a single leaf at offsetBytes.
template<typename Leaf, typename Incomplete>
void forEachInitStorageUnit(const type::Type& t, int offsetBytes, Leaf&& leaf,
        Incomplete&& incompleteArrayError) {
    if (t.isUnion() || t.isStructure()) {
        fillStorageUnitsBySize(offsetBytes, t.getSize(), leaf);
        return;
    }
    if (t.isArray()) {
        const int n = t.getArraySize();
        if (n <= 0) {
            incompleteArrayError();
            return;
        }
        // Prefer declared size (includes trailing padding / stride packing).
        fillStorageUnitsBySize(offsetBytes, t.getSize(), leaf);
        return;
    }
    leaf(offsetBytes, t);
}

} // namespace semantic_analyzer

#endif
