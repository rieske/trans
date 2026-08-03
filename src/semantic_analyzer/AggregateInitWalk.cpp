#include "AggregateInitWalk.h"

#include "AggregateDesignatorPath.h"

#include "ast/InitializerListExpression.h"

#include <vector>

namespace semantic_analyzer {
namespace {

void incompleteArrayError(AggregateInitSink& sink) {
    sink.error(kIncompleteArrayInitMsg);
}

struct AggregateSlots {
    const type::Type& type;
    int baseOffset { 0 };
    bool isArray { false };

    int count() const {
        return isArray ? type.getArraySize() : type.memberCount();
    }

    const char* excessMessage() const {
        return isArray ? "excess elements in array initializer"
                       : "excess elements in structure initializer";
    }

    bool slotAt(int i, type::Type& outType, int& outAbsOffset) const {
        if (isArray) {
            if (i < 0 || i >= type.getArraySize()) {
                return false;
            }
            outType = type.getElementType();
            outAbsOffset = baseOffset + i * type.getElementStride();
            return true;
        }
        std::string name;
        int rel = 0;
        if (!type.memberAt(i, name, outType, rel)) {
            return false;
        }
        outAbsOffset = baseOffset + rel;
        return true;
    }
};

// How placeSlotsFrom treats a designated (or exhausted) stream element mid-loop:
// ZeroThrough - keep calling placeIntoSlot (zeros remaining nested slots; ei stays put).
// StopEarly  - leave remaining slots for outer written[] residual; preserve ei for designator.
enum class DesignatedBarrier { ZeroThrough, StopEarly };

struct WrittenMarkCtx {
    std::vector<bool>* written;
    int nSlots;
};

std::size_t placeIntoSlot(const type::Type& slotType, int slotOff,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei,
        AggregateInitSink& sink);
std::size_t placeSlotsFrom(const AggregateSlots& slots, int startIndex, bool noSiblingResume,
        DesignatedBarrier barrier, const std::vector<ast::InitializerElement>& elements,
        std::size_t ei, AggregateInitSink& sink, WrittenMarkCtx* markCtx, bool markTopLevel);

bool tryPlaceWholeAggregate(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    if (!value || !placeType.isAggregate()) {
        return false;
    }
    if (sink.placeStringArray(offsetBytes, placeType, value)) {
        return true;
    }
    return sink.placeAggregateCopy(offsetBytes, placeType, value);
}

// Place a single expression into a type at offset.
// Whole string/copy is tried before peel so designator elision into nested char[] works.
void placeValue(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    if (!sink.ok()) {
        return;
    }
    if (auto* nestedList = value ? dynamic_cast<ast::InitializerListExpression*>(value) : nullptr) {
        if (placeType.isAggregate()) {
            walkAggregateInit(placeType, nestedList, offsetBytes, sink);
            return;
        }
    }
    if (placeType.isAggregate()) {
        if (!value) {
            sink.onUnwritten(offsetBytes, placeType);
            return;
        }
        if (tryPlaceWholeAggregate(placeType, offsetBytes, value, sink)) {
            return;
        }
        // Unions: residual whole storage, then peel first arm.
        if (placeType.isUnion()) {
            sink.onUnwritten(offsetBytes, placeType);
            if (!sink.ok()) {
                return;
            }
            AggregateSlots arms { placeType, offsetBytes, false };
            type::Type first = type::voidType();
            int off = 0;
            if (!arms.slotAt(0, first, off)) {
                return;
            }
            placeValue(first, off, value, sink);
            return;
        }
        // Structure/array elision: full-object zero (padding), then first subobject only.
        const bool isArray = placeType.isArray();
        const int n = isArray ? placeType.getArraySize() : placeType.memberCount();
        if (n <= 0) {
            if (isArray) {
                incompleteArrayError(sink);
            } else {
                sink.error("excess elements in structure initializer");
            }
            return;
        }
        AggregateSlots slots { placeType, offsetBytes, isArray };
        type::Type first = type::voidType();
        int off = 0;
        if (!slots.slotAt(0, first, off)) {
            return;
        }
        // Padding already cleared by walkSlottedAggregate's full-object zero when
        // this elision runs under brace init; only place the first subobject.
        placeValue(first, off, value, sink);
        return;
    }
    if (value) {
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
        while (nested) {
            if (nested->getElements().size() > 1) {
                sink.error("excess elements in scalar initializer");
                return;
            }
            if (nested->getElements().empty() || !nested->getElements().front().value) {
                sink.onUnwritten(offsetBytes, placeType);
                return;
            }
            value = nested->getElements().front().value.get();
            nested = dynamic_cast<ast::InitializerListExpression*>(value);
        }
        sink.placeScalar(offsetBytes, placeType, value);
    } else {
        sink.onUnwritten(offsetBytes, placeType);
    }
}

// Stream element at ei into slotType. Barrier leaves ei unchanged; a present
// non-designated value always advances ei (uniform progress contract).
std::size_t placeIntoSlot(const type::Type& slotType, int slotOff,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei,
        AggregateInitSink& sink) {
    if (ei >= elements.size() || !elements[ei].value || elements[ei].isDesignated()) {
        sink.onUnwritten(slotOff, slotType);
        return ei;
    }
    const std::size_t start = ei;
    auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
    if (nested && slotType.isAggregate()) {
        walkAggregateInit(slotType, nested, slotOff, sink);
        return ei + 1;
    }

    // One whole-copy/string attempt for any aggregate, then branch.
    if (slotType.isAggregate()
            && tryPlaceWholeAggregate(slotType, slotOff, elements[ei].value.get(), sink)) {
        return ei + 1;
    }

    if (slotType.isStructure() || slotType.isArray()) {
        const int n = slotType.isArray() ? slotType.getArraySize() : slotType.memberCount();
        if (n <= 0) {
            if (slotType.isArray()) {
                incompleteArrayError(sink);
            } else {
                sink.error("excess elements in structure initializer");
            }
            return ei + 1;
        }
        ei = placeSlotsFrom(AggregateSlots { slotType, slotOff, slotType.isArray() },
                /*startIndex=*/0, /*noSiblingResume=*/false, DesignatedBarrier::ZeroThrough,
                elements, ei, sink, nullptr, false);
    } else if (slotType.isUnion()) {
        sink.onUnwritten(slotOff, slotType);
        AggregateSlots arms { slotType, slotOff, false };
        type::Type first = type::voidType();
        int off = 0;
        if (arms.slotAt(0, first, off)) {
            ei = placeIntoSlot(first, off, elements, ei, sink);
        } else {
            // Empty union: residual already applied; consume value without re-zero.
            ++ei;
        }
    } else {
        placeValue(slotType, slotOff, elements[ei].value.get(), sink);
        ei = ei + 1;
    }

    // Present non-designated value must be consumed (loop safety).
    if (ei == start) {
        if (sink.ok()) {
            sink.onUnwritten(slotOff, slotType);
        }
        ++ei;
    }
    return ei;
}

void markWrittenSlot(WrittenMarkCtx* m, int idx) {
    if (m && m->written && idx >= 0 && idx < m->nSlots) {
        (*m->written)[static_cast<std::size_t>(idx)] = true;
    }
}

// Single slot loop for stream elision (ZeroThrough) and designator resume (StopEarly).
std::size_t placeSlotsFrom(const AggregateSlots& slots, int startIndex, bool noSiblingResume,
        DesignatedBarrier barrier, const std::vector<ast::InitializerElement>& elements,
        std::size_t ei, AggregateInitSink& sink, WrittenMarkCtx* markCtx, bool markTopLevel) {
    const int n = slots.count();
    if (slots.isArray && n <= 0) {
        incompleteArrayError(sink);
        return ei;
    }
    const int last = noSiblingResume ? startIndex : (n - 1);
    for (int i = startIndex; i <= last && i < n && sink.ok(); ++i) {
        if (barrier == DesignatedBarrier::StopEarly
                && (ei >= elements.size() || elements[ei].isDesignated())) {
            return ei;
        }
        if (markTopLevel) {
            markWrittenSlot(markCtx, i);
        }
        type::Type slotType = type::voidType();
        int slotOff = 0;
        if (!slots.slotAt(i, slotType, slotOff)) {
            break;
        }
        ei = placeIntoSlot(slotType, slotOff, elements, ei, sink);
    }
    return ei;
}

// Path resume: navigate to the designated container, then AggregateSlots + placeIntoSlot.
std::size_t fillFromPathAt(const type::Type& container, int containerOff,
        const std::vector<DesignatorPathItem>& path, std::size_t depth,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei,
        AggregateInitSink& sink, WrittenMarkCtx* markCtx) {
    if (!sink.ok() || depth >= path.size()) {
        return ei;
    }
    const DesignatorPathItem& item = path[depth];
    const bool markTop = (depth == 0);

    if (item.isArray) {
        const int n = container.getArraySize();
        if (n <= 0) {
            incompleteArrayError(sink);
            return ei;
        }
        const int stride = container.getElementStride();
        const type::Type elem = container.getElementType();
        if (depth + 1 == path.size()) {
            AggregateSlots slots { container, containerOff, true };
            return placeSlotsFrom(slots, item.index, /*noSiblingResume=*/false,
                    DesignatedBarrier::StopEarly, elements, ei, sink, markCtx, markTop);
        }
        if (markTop) {
            markWrittenSlot(markCtx, item.index);
        }
        ei = fillFromPathAt(elem, containerOff + item.index * stride, path, depth + 1, elements, ei,
                sink, markCtx);
        AggregateSlots slots { container, containerOff, true };
        return placeSlotsFrom(slots, item.index + 1, /*noSiblingResume=*/false,
                DesignatedBarrier::StopEarly, elements, ei, sink, markCtx, markTop);
    }

    // Structure / union member path.
    if (depth + 1 == path.size()) {
        AggregateSlots slots { container, containerOff, false };
        return placeSlotsFrom(slots, item.index, container.isUnion(), DesignatedBarrier::StopEarly,
                elements, ei, sink, markCtx, markTop);
    }
    std::string name;
    type::Type memberType = type::voidType();
    int moff = 0;
    if (!container.memberAt(item.index, name, memberType, moff)) {
        return ei;
    }
    if (markTop) {
        markWrittenSlot(markCtx, item.index);
    }
    ei = fillFromPathAt(memberType, containerOff + moff, path, depth + 1, elements, ei, sink,
            markCtx);
    if (container.isUnion()) {
        return ei;
    }
    AggregateSlots slots { container, containerOff, false };
    return placeSlotsFrom(slots, item.index + 1, /*noSiblingResume=*/false,
            DesignatedBarrier::StopEarly, elements, ei, sink, markCtx, markTop);
}

void applyDesignator(const type::Type& targetType, int baseOffset,
        const std::vector<ast::InitializerElement>& src, std::size_t& ei,
        std::vector<bool>* written, int nSlots, bool isArrayRoot, AggregateInitSink& sink) {
    if (ei >= src.size()) {
        return;
    }
    const auto& el = src[ei];
    std::vector<ast::DesignatorStep> steps;
    std::string foldErr;
    if (!foldDesignatorSteps(el, steps, foldErr)) {
        sink.error(foldErr);
        ++ei;
        return;
    }
    type::Type placeType = type::voidType();
    int placeOff = 0;
    std::vector<DesignatorPathItem> path;
    int firstIdx = -1;
    std::string err;
    if (!resolveDesignator(targetType, baseOffset, steps, placeType, placeOff, path, firstIdx, err)) {
        sink.error(err);
        ++ei;
        return;
    }
    // Full-object onUnwritten at walk entry already zeros holes; do not re-zero
    // the designated outer slot before nested placement (avoids double zero).
    WrittenMarkCtx markCtx { written, nSlots };
    (void)isArrayRoot;
    markWrittenSlot(&markCtx, firstIdx);

    // Single placement entry: placeValue tries whole-copy/string then peel/list.
    placeValue(placeType, placeOff, el.value.get(), sink);
    ++ei;

    if (advanceDesignatorPath(path, targetType) && !path.empty()) {
        ei = fillFromPathAt(targetType, baseOffset, path, 0, src, ei, sink, &markCtx);
    }
}

void walkSlottedAggregate(const AggregateSlots& slots, const std::vector<ast::InitializerElement>& src,
        const type::Type& rootType, AggregateInitSink& sink) {
    const int n = slots.count();
    if (slots.isArray && n <= 0) {
        incompleteArrayError(sink);
        return;
    }
    // C 6.7.9: unmentioned members and padding are zero as if static storage.
    // Zero the whole object first (covers inter-member padding that member-wise
    // onUnwritten misses), then apply explicit initializers on top.
    // git: struct object_info empty = { 0 }; memcmp against BSS zero in ref-filter.
    sink.onUnwritten(slots.baseOffset, rootType);
    if (!sink.ok()) {
        return;
    }

    std::vector<bool> written(static_cast<std::size_t>(n > 0 ? n : 0), false);
    std::size_t ei = 0;
    int positional = 0;

    while (ei < src.size() && sink.ok()) {
        const auto& el = src[ei];
        if (!el.value) {
            ++ei;
            continue;
        }
        if (el.isDesignated()) {
            applyDesignator(rootType, slots.baseOffset, src, ei, &written, n, slots.isArray, sink);
            positional = n;
            continue;
        }
        if (positional >= n) {
            sink.error(slots.excessMessage());
            ++ei;
            continue;
        }
        type::Type slotType = type::voidType();
        int slotOff = 0;
        if (!slots.slotAt(positional, slotType, slotOff)) {
            break;
        }
        ei = placeIntoSlot(slotType, slotOff, src, ei, sink);
        written[static_cast<std::size_t>(positional)] = true;
        ++positional;
    }
    // Remainder already zeroed by the full-object onUnwritten above.
}

void walkUnionAggregate(const type::Type& targetType, int baseOffset,
        const std::vector<ast::InitializerElement>& src, AggregateInitSink& sink) {
    if (src.empty() || !src.front().value) {
        sink.onUnwritten(baseOffset, targetType);
        return;
    }
    sink.onUnwritten(baseOffset, targetType);
    bool sawPositional = false;
    bool sawDesignator = false;
    std::size_t ei = 0;
    while (ei < src.size() && sink.ok()) {
        const auto& el = src[ei];
        if (!el.value) {
            ++ei;
            continue;
        }
        if (el.isDesignated()) {
            sawDesignator = true;
            applyDesignator(targetType, baseOffset, src, ei, nullptr, 0, false, sink);
            continue;
        }
        if (sawPositional || sawDesignator) {
            sink.error("excess elements in union initializer");
            ++ei;
            continue;
        }
        sawPositional = true;
        AggregateSlots arms { targetType, baseOffset, false };
        type::Type first = type::voidType();
        int off = 0;
        if (!arms.slotAt(0, first, off)) {
            ++ei;
            continue;
        }
        ei = placeIntoSlot(first, off, src, ei, sink);
    }
}

} // namespace

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink) {
    if (!sink.ok() || !list) {
        return;
    }
    const auto& src = list->getElements();
    if (targetType.isUnion()) {
        walkUnionAggregate(targetType, baseOffset, src, sink);
        return;
    }
    if (targetType.isStructure() || targetType.isArray()) {
        AggregateSlots slots { targetType, baseOffset, targetType.isArray() };
        walkSlottedAggregate(slots, src, targetType, sink);
        return;
    }
    sink.error("brace initializer for non-aggregate type");
}

} // namespace semantic_analyzer
