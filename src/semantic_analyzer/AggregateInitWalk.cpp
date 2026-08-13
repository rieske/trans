#include "AggregateInitWalk.h"

#include "AggregateDesignatorPath.h"

#include "ast/InitializerListExpression.h"
#include "types/TypeQuery.h"

#include <functional>
#include <optional>
#include <vector>

namespace semantic_analyzer {
namespace {

type::FoundMember place(const type::Type& t, int offsetBytes) {
    return type::FoundMember { "", t, offsetBytes, {} };
}

std::optional<type::FoundMember> firstSubobjectOf(const type::Type& t, int baseOffset, AggregateInitSink& sink) {
    if (t.isRecord()) {
        if (auto first = type::memberAt(t, 0)) {
            return first->atBase(baseOffset);
        }
        return std::nullopt;
    }
    if (t.isArray()) {
        if (t.getArraySize() <= 0) {
            sink.error("array brace initializers for incomplete arrays are not implemented");
            return std::nullopt;
        }
        return place(t.getElementType(), baseOffset);
    }
    return std::nullopt;
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

    std::optional<type::FoundMember> slotAt(int i) const {
        if (isArray) {
            if (i < 0 || i >= type.getArraySize()) {
                return std::nullopt;
            }
            return place(type.getElementType(), baseOffset + i * type.getElementStride());
        }
        if (auto member = type::memberAt(type, i)) {
            return member->atBase(baseOffset);
        }
        return std::nullopt;
    }
};

void placeAt(const type::FoundMember& slot, ast::Expression* value, AggregateInitSink& sink);

std::size_t fillFromStream(const type::Type& destType, int baseOffset,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink);

std::size_t fillFromPath(const type::Type& root, int baseOffset, std::vector<DesignatorPathItem> path,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink,
        const std::function<void(int)>& markTopLevel);

void placeAt(const type::FoundMember& slot, ast::Expression* value, AggregateInitSink& sink) {
    if (!sink.ok()) {
        return;
    }
    if (auto* nestedList = value ? dynamic_cast<ast::InitializerListExpression*>(value) : nullptr) {
        if (slot.type.isAggregate()) {
            walkAggregateInit(slot.type, nestedList, slot.offsetBytes, sink);
            return;
        }
    }
    if (slot.type.isRecord() && value) {
        // Whole nested record from a compatible expression (.needle = *want).
        // Otherwise current-object: scalar initializes the first subobject (.in = 5).
        const type::Type src = type::afterLvalueConversion(value->getType());
        if (type::productAssignFrom(slot.type, src)) {
            sink.placeScalar(slot, value);
            return;
        }
        auto first = firstSubobjectOf(slot.type, slot.offsetBytes, sink);
        if (!first) {
            return;
        }
        placeAt(*first, value, sink);
        return;
    }
    if (slot.type.isAggregate()) {
        if (value) {
            auto first = firstSubobjectOf(slot.type, slot.offsetBytes, sink);
            if (!first) {
                return;
            }
            placeAt(*first, value, sink);
            return;
        }
        sink.onUnwritten(slot);
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
                sink.onUnwritten(slot);
                return;
            }
            value = nested->getElements().front().value.get();
            nested = dynamic_cast<ast::InitializerListExpression*>(value);
        }
        sink.placeScalar(slot, value);
    } else {
        sink.onUnwritten(slot);
    }
}

std::size_t fillSlottedFromStream(const AggregateSlots& slots,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink) {
    const int n = slots.count();
    if (slots.isArray && n <= 0) {
        sink.error("array brace initializers for incomplete arrays are not implemented");
        return ei;
    }
    for (int i = 0; i < n && sink.ok(); ++i) {
        auto slot = slots.slotAt(i);
        if (!slot) {
            break;
        }
        if (ei >= elements.size() || !elements[ei].value || elements[ei].isDesignated()) {
            sink.onUnwritten(*slot);
            continue;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
        if (nested && slot->type.isAggregate()) {
            walkAggregateInit(slot->type, nested, slot->offsetBytes, sink);
            ++ei;
            continue;
        }
        if (slot->type.isAggregate() && !nested) {
            ei = fillFromStream(slot->type, slot->offsetBytes, elements, ei, sink);
            continue;
        }
        placeAt(*slot, elements[ei].value.get(), sink);
        ++ei;
    }
    return ei;
}

std::size_t fillFromStream(const type::Type& destType, int baseOffset,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink) {
    if (!sink.ok()) {
        return ei;
    }
    auto isBarrier = [&](std::size_t i) {
        return i >= elements.size() || !elements[i].value || elements[i].isDesignated();
    };

    if (destType.isUnion()) {
        if (isBarrier(ei)) {
            sink.onUnwritten(place(destType, baseOffset));
            return ei;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
        if (nested) {
            walkAggregateInit(destType, nested, baseOffset, sink);
            return ei + 1;
        }
        sink.onUnwritten(place(destType, baseOffset));
        auto first = firstSubobjectOf(destType, baseOffset, sink);
        if (!first) {
            return ei + 1;
        }
        if (first->type.isAggregate()) {
            return fillFromStream(first->type, first->offsetBytes, elements, ei, sink);
        }
        placeAt(*first, elements[ei].value.get(), sink);
        return ei + 1;
    }

    if (destType.isStructure() || destType.isArray()) {
        if (!isBarrier(ei)) {
            auto* whole = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (whole) {
                walkAggregateInit(destType, whole, baseOffset, sink);
                return ei + 1;
            }
        }
        AggregateSlots slots { destType, baseOffset, destType.isArray() };
        return fillSlottedFromStream(slots, elements, ei, sink);
    }

    if (isBarrier(ei)) {
        sink.onUnwritten(place(destType, baseOffset));
        return ei;
    }
    placeAt(place(destType, baseOffset), elements[ei].value.get(), sink);
    return ei + 1;
}

std::size_t fillFromPath(const type::Type& root, int baseOffset, std::vector<DesignatorPathItem> path,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink,
        const std::function<void(int)>& markTopLevel) {
    if (!sink.ok() || path.empty()) {
        return ei;
    }
    std::function<std::size_t(const type::Type&, int, std::size_t, int)> rec;
    rec = [&](const type::Type& container, int containerOff, std::size_t depth, int topLevelHint) -> std::size_t {
        if (!sink.ok() || depth >= path.size()) {
            return ei;
        }
        const DesignatorPathItem& item = path[depth];
        if (item.isArray) {
            const int n = container.getArraySize();
            if (n <= 0) {
                sink.error("array brace initializers for incomplete arrays are not implemented");
                return ei;
            }
            const int stride = container.getElementStride();
            const type::Type elem = container.getElementType();
            if (depth + 1 == path.size()) {
                for (int i = item.index; i < n && sink.ok(); ++i) {
                    if (ei >= elements.size() || elements[ei].isDesignated()) {
                        return ei;
                    }
                    if (topLevelHint >= 0 && depth == 0) {
                        markTopLevel(i);
                    }
                    const std::size_t before = ei;
                    ei = fillFromStream(elem, containerOff + i * stride, elements, ei, sink);
                    if (ei == before && ei < elements.size() && !elements[ei].isDesignated()) {
                        sink.onUnwritten(place(elem, containerOff + i * stride));
                        ++ei;
                    }
                }
                return ei;
            }
            if (topLevelHint >= 0 && depth == 0) {
                markTopLevel(item.index);
            }
            ei = rec(elem, containerOff + item.index * stride, depth + 1, -1);
            for (int i = item.index + 1; i < n && sink.ok(); ++i) {
                if (ei >= elements.size() || elements[ei].isDesignated()) {
                    return ei;
                }
                if (topLevelHint >= 0 && depth == 0) {
                    markTopLevel(i);
                }
                ei = fillFromStream(elem, containerOff + i * stride, elements, ei, sink);
            }
            return ei;
        }

        // Structure / union: no sibling resume into other union arms.
        const int nMembers = container.memberCount();
        const bool noSiblingResume = container.isUnion();
        auto consumeMember = [&](int mi) -> std::size_t {
            auto member = type::memberAt(container, mi);
            if (!member) {
                return ei;
            }
            type::FoundMember slot = member->atBase(containerOff);
            if (ei >= elements.size() || elements[ei].isDesignated()) {
                return ei;
            }
            if (topLevelHint >= 0 && depth == 0) {
                markTopLevel(mi);
            }
            if (slot.type.isAggregate()) {
                return fillFromStream(slot.type, slot.offsetBytes, elements, ei, sink);
            }
            placeAt(slot, elements[ei].value.get(), sink);
            return ei + 1;
        };
        if (depth + 1 == path.size()) {
            const int lastMi = noSiblingResume ? item.index : (nMembers - 1);
            for (int mi = item.index; mi <= lastMi && mi < nMembers && sink.ok(); ++mi) {
                const std::size_t before = ei;
                ei = consumeMember(mi);
                if (ei == before) {
                    return ei;
                }
            }
            return ei;
        }
        auto intoMember = type::memberAt(container, item.index);
        if (!intoMember) {
            return ei;
        }
        type::FoundMember into = intoMember->atBase(containerOff);
        if (topLevelHint >= 0 && depth == 0) {
            markTopLevel(item.index);
        }
        ei = rec(into.type, into.offsetBytes, depth + 1, -1);
        if (noSiblingResume) {
            return ei;
        }
        for (int mi = item.index + 1; mi < nMembers && sink.ok(); ++mi) {
            const std::size_t before = ei;
            ei = consumeMember(mi);
            if (ei == before) {
                return ei;
            }
        }
        return ei;
    };
    return rec(root, baseOffset, 0, 0);
}

void walkSlottedAggregate(const AggregateSlots& slots, const std::vector<ast::InitializerElement>& src,
        AggregateInitSink& sink,
        const std::function<void(std::size_t&, std::vector<bool>*, int, bool)>& applyDesignator) {
    const int n = slots.count();
    if (slots.isArray && n <= 0) {
        sink.error("array brace initializers for incomplete arrays are not implemented");
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
            applyDesignator(ei, &written, n, slots.isArray);
            positional = n;
            continue;
        }
        if (positional >= n) {
            sink.error(slots.excessMessage());
            ++ei;
            continue;
        }
        auto slot = slots.slotAt(positional);
        if (!slot) {
            break;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
        if (nested && slot->type.isAggregate()) {
            written[static_cast<std::size_t>(positional)] = true;
            walkAggregateInit(slot->type, nested, slot->offsetBytes, sink);
            ++ei;
            ++positional;
            continue;
        }
        if (slot->type.isAggregate() && !nested) {
            const std::size_t before = ei;
            ei = fillFromStream(slot->type, slot->offsetBytes, src, ei, sink);
            if (ei == before) {
                sink.onUnwritten(*slot);
            }
            written[static_cast<std::size_t>(positional)] = true;
            ++positional;
            continue;
        }
        written[static_cast<std::size_t>(positional)] = true;
        placeAt(*slot, el.value.get(), sink);
        ++ei;
        ++positional;
    }
    for (int i = 0; i < n && sink.ok(); ++i) {
        if (!written[static_cast<std::size_t>(i)]) {
            if (auto slot = slots.slotAt(i)) {
                sink.onUnwritten(*slot);
            }
        }
    }
}

} // namespace

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink) {
    if (!sink.ok() || !list) {
        return;
    }
    const auto& src = list->getElements();

    auto applyDesignator = [&](std::size_t& ei, std::vector<bool>* written, int nSlots, bool isArrayRoot) {
        const auto& el = src[ei];
        std::vector<ast::DesignatorStep> steps;
        std::string foldErr;
        if (!foldDesignatorSteps(el, steps, foldErr)) {
            sink.error(foldErr);
            return;
        }
        type::FoundMember dest;
        std::vector<DesignatorPathItem> path;
        int firstIdx = -1;
        std::string err;
        if (!resolveDesignator(targetType, baseOffset, steps, dest, path, firstIdx, err)) {
            sink.error(err);
            ++ei;
            return;
        }
        auto mark = [&](int idx) {
            if (written && idx >= 0 && idx < nSlots) {
                (*written)[static_cast<std::size_t>(idx)] = true;
            }
        };
        if (firstIdx >= 0 && path.size() > 1 && written
                && firstIdx < nSlots && !(*written)[static_cast<std::size_t>(firstIdx)]) {
            if (isArrayRoot) {
                const int stride = targetType.getElementStride();
                sink.onUnwritten(place(targetType.getElementType(),
                        baseOffset + firstIdx * stride));
            } else if (auto first = type::memberAt(targetType, firstIdx)) {
                sink.onUnwritten(first->atBase(baseOffset));
            }
        }
        mark(firstIdx);

        auto* nestedValue = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
        if (nestedValue) {
            walkAggregateInit(dest.type, nestedValue, dest.offsetBytes, sink);
            ++ei;
        } else if (dest.type.isAggregate()) {
            sink.onUnwritten(dest);
            placeAt(dest, el.value.get(), sink);
            ++ei;
        } else {
            placeAt(dest, el.value.get(), sink);
            ++ei;
        }
        if (advanceDesignatorPath(path, targetType)) {
            ei = fillFromPath(targetType, baseOffset, path, src, ei, sink, mark);
        }
    };

    if (targetType.isUnion()) {
        if (src.empty() || !src.front().value) {
            sink.onUnwritten(place(targetType, baseOffset));
            return;
        }
        sink.onUnwritten(place(targetType, baseOffset));
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
                applyDesignator(ei, nullptr, 0, false);
                continue;
            }
            if (sawPositional || sawDesignator) {
                sink.error("excess elements in union initializer");
                ++ei;
                continue;
            }
            sawPositional = true;
            auto first = firstSubobjectOf(targetType, baseOffset, sink);
            if (!first) {
                ++ei;
                continue;
            }
            const std::size_t before = ei;
            if (first->type.isAggregate()) {
                ei = fillFromStream(first->type, first->offsetBytes, src, ei, sink);
            } else {
                placeAt(*first, src[ei].value.get(), sink);
                ++ei;
            }
            if (ei == before) {
                ++ei;
            }
        }
        return;
    }

    if (targetType.isStructure() || targetType.isArray()) {
        AggregateSlots slots { targetType, baseOffset, targetType.isArray() };
        walkSlottedAggregate(slots, src, sink, applyDesignator);
        return;
    }
    sink.error("brace initializer for non-aggregate type");
}

} // namespace semantic_analyzer
