#include "AggregateInitWalk.h"

#include "AggregateDesignatorPath.h"
#include "CharArrayStringInit.h"

#include "ast/InitializerListExpression.h"
#include "types/TypeQuery.h"

#include <optional>
#include <string>
#include <vector>

namespace semantic_analyzer {
namespace {

void incompleteArrayError(AggregateInitSink& sink) {
    sink.error(kIncompleteArrayInitMsg);
}

bool reportEmptyAggregate(const type::Type& type, AggregateInitSink& sink) {
    if (type.isArray() && type.getArraySize() <= 0) {
        incompleteArrayError(sink);
        return true;
    }
    return false;
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
            return type::FoundMember { "", type.getElementType(),
                    baseOffset + i * type.getElementStride(), {} };
        }
        auto member = type::memberAt(type, i);
        if (!member) {
            return std::nullopt;
        }
        return member->atBase(baseOffset);
    }
};

std::size_t consumeSlot(const type::Type& slotType, int slotOff,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei,
        AggregateInitSink& sink);
std::size_t placeSlotsFrom(const AggregateSlots& slots, int startIndex, bool noSiblingResume,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink);

bool tryPlaceCharArrayString(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    if (!isCharArrayStringInit(placeType, value)) {
        return false;
    }
    if (!sink.placeStringArray(offsetBytes, placeType, value)) {
        sink.error("string literal initializer requires character array");
    }
    return true;
}

bool tryPlaceComplete(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    if (tryPlaceCharArrayString(placeType, offsetBytes, value, sink)) {
        return true;
    }
    if (auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
            nested && placeType.isAggregate()) {
        walkAggregateInit(placeType, nested, offsetBytes, sink);
        return true;
    }
    return placeType.isAggregate() && value
            && sink.placeAggregateCopy(offsetBytes, placeType, value);
}

void placeFirstSlot(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    const bool isArray = placeType.isArray();
    if (isArray && reportEmptyAggregate(placeType, sink)) {
        return;
    }
    if (placeType.isStructure() && placeType.memberCount() <= 0) {
        sink.error("excess elements in structure initializer");
        return;
    }
    AggregateSlots slots { placeType, offsetBytes, isArray };
    auto first = slots.slotAt(0);
    if (!first) {
        return;
    }
    if (first->isBitField()) {
        sink.placeScalar(first->offsetBytes, first->type, value, first->bitField);
    } else {
        placeAt(first->type, first->offsetBytes, value, sink);
    }
}

} // namespace

void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    if (!sink.ok() || !value) {
        return;
    }
    if (tryPlaceComplete(placeType, offsetBytes, value, sink)) {
        return;
    }
    if (placeType.isAggregate()) {
        placeFirstSlot(placeType, offsetBytes, value, sink);
        return;
    }
    auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
    while (nested) {
        if (nested->getElements().size() > 1) {
            sink.error("excess elements in scalar initializer");
            return;
        }
        if (nested->getElements().empty() || !nested->getElements().front().value) {
            return;
        }
        value = nested->getElements().front().value.get();
        nested = dynamic_cast<ast::InitializerListExpression*>(value);
    }
    sink.placeScalar(offsetBytes, placeType, value);
}

namespace {

std::size_t consumeSlot(const type::Type& slotType, int slotOff,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei,
        AggregateInitSink& sink) {
    if (ei >= elements.size() || !elements[ei].value || elements[ei].isDesignated()) {
        return ei;
    }
    ast::Expression* value = elements[ei].value.get();
    if (tryPlaceComplete(slotType, slotOff, value, sink)) {
        return ei + 1;
    }
    if (slotType.isStructure() && slotType.memberCount() <= 0) {
        return ei;
    }
    if (slotType.isStructure() || slotType.isArray()) {
        if (reportEmptyAggregate(slotType, sink)) {
            return ei + 1;
        }
        return placeSlotsFrom(AggregateSlots { slotType, slotOff, slotType.isArray() },
                /*startIndex=*/0, /*noSiblingResume=*/false, elements, ei, sink);
    }
    if (slotType.isUnion()) {
        AggregateSlots arms { slotType, slotOff, false };
        if (auto first = arms.slotAt(0)) {
            if (first->isBitField()) {
                sink.placeScalar(first->offsetBytes, first->type, value, first->bitField);
                return ei + 1;
            }
            return consumeSlot(first->type, first->offsetBytes, elements, ei, sink);
        }
        return ei + 1;
    }
    placeAt(slotType, slotOff, value, sink);
    return ei + 1;
}

std::size_t placeSlotsFrom(const AggregateSlots& slots, int startIndex, bool noSiblingResume,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink) {
    const int n = slots.count();
    if (reportEmptyAggregate(slots.type, sink)) {
        return ei;
    }
    const int last = noSiblingResume ? startIndex : (n - 1);
    for (int i = startIndex; i <= last && i < n && sink.ok(); ++i) {
        if (ei >= elements.size() || elements[ei].isDesignated()) {
            return ei;
        }
        auto slot = slots.slotAt(i);
        if (!slot) {
            break;
        }
        if (slot->isBitField()) {
            if (ei >= elements.size() || !elements[ei].value || elements[ei].isDesignated()) {
                sink.placeScalar(slot->offsetBytes, slot->type, nullptr, slot->bitField);
            } else {
                sink.placeScalar(slot->offsetBytes, slot->type, elements[ei].value.get(),
                        slot->bitField);
                ++ei;
            }
        } else {
            ei = consumeSlot(slot->type, slot->offsetBytes, elements, ei, sink);
        }
    }
    return ei;
}

std::size_t fillFromPathAt(const type::Type& container, int containerOff,
        const std::vector<DesignatorPathItem>& path, std::size_t depth,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei,
        AggregateInitSink& sink) {
    if (!sink.ok() || depth >= path.size()) {
        return ei;
    }
    const DesignatorPathItem& item = path[depth];

    if (item.isArray) {
        if (reportEmptyAggregate(container, sink)) {
            return ei;
        }
        const int stride = container.getElementStride();
        const type::Type elem = container.getElementType();
        if (depth + 1 == path.size()) {
            AggregateSlots slots { container, containerOff, true };
            return placeSlotsFrom(slots, item.index, /*noSiblingResume=*/false, elements, ei, sink);
        }
        ei = fillFromPathAt(elem, containerOff + item.index * stride, path, depth + 1, elements, ei,
                sink);
        AggregateSlots slots { container, containerOff, true };
        return placeSlotsFrom(slots, item.index + 1, /*noSiblingResume=*/false, elements, ei, sink);
    }

    if (depth + 1 == path.size()) {
        AggregateSlots slots { container, containerOff, false };
        return placeSlotsFrom(slots, item.index, container.isUnion(), elements, ei, sink);
    }
    auto member = type::memberAt(container, item.index);
    if (!member) {
        return ei;
    }
    ei = fillFromPathAt(member->type, containerOff + member->offsetBytes, path, depth + 1, elements,
            ei, sink);
    if (container.isUnion()) {
        return ei;
    }
    AggregateSlots slots { container, containerOff, false };
    return placeSlotsFrom(slots, item.index + 1, /*noSiblingResume=*/false, elements, ei, sink);
}

void applyDesignator(const type::Type& targetType, int baseOffset,
        const std::vector<ast::InitializerElement>& src, std::size_t& ei, AggregateInitSink& sink) {
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
    type::FoundMember place {};
    std::vector<DesignatorPathItem> path;
    int firstIdx = -1;
    std::string err;
    if (!resolveDesignator(targetType, baseOffset, steps, place, path, firstIdx, err)) {
        sink.error(err);
        ++ei;
        return;
    }
    if (place.isBitField()) {
        sink.placeScalar(place.offsetBytes, place.type, el.value.get(), place.bitField);
    } else {
        placeAt(place.type, place.offsetBytes, el.value.get(), sink);
    }
    ++ei;

    if (advanceDesignatorPath(path, targetType) && !path.empty()) {
        ei = fillFromPathAt(targetType, baseOffset, path, 0, src, ei, sink);
    }
}

void walkSlottedAggregate(const AggregateSlots& slots, const std::vector<ast::InitializerElement>& src,
        const type::Type& rootType, AggregateInitSink& sink) {
    const int n = slots.count();
    if (reportEmptyAggregate(slots.type, sink)) {
        return;
    }
    sink.onUnwritten(slots.baseOffset, rootType);
    if (!sink.ok()) {
        return;
    }

    std::size_t ei = 0;
    int positional = 0;

    while (ei < src.size() && sink.ok()) {
        const auto& el = src[ei];
        if (!el.value) {
            ++ei;
            continue;
        }
        if (el.isDesignated()) {
            applyDesignator(rootType, slots.baseOffset, src, ei, sink);
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
        if (slot->isBitField()) {
            sink.placeScalar(slot->offsetBytes, slot->type, el.value.get(), slot->bitField);
            ++ei;
        } else {
            ei = consumeSlot(slot->type, slot->offsetBytes, src, ei, sink);
        }
        ++positional;
    }
}

void walkUnionAggregate(const type::Type& targetType, int baseOffset,
        const std::vector<ast::InitializerElement>& src, AggregateInitSink& sink) {
    if (src.empty() || !src.front().value) {
        sink.onUnwritten(baseOffset, targetType);
        return;
    }
    sink.onUnwritten(baseOffset, targetType);
    if (!sink.ok()) {
        return;
    }
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
            applyDesignator(targetType, baseOffset, src, ei, sink);
            continue;
        }
        if (sawPositional || sawDesignator) {
            sink.error("excess elements in union initializer");
            ++ei;
            continue;
        }
        sawPositional = true;
        AggregateSlots arms { targetType, baseOffset, false };
        auto first = arms.slotAt(0);
        if (!first) {
            ++ei;
            continue;
        }
        if (first->isBitField()) {
            sink.placeScalar(first->offsetBytes, first->type, el.value.get(), first->bitField);
            ++ei;
        } else {
            ei = consumeSlot(first->type, first->offsetBytes, src, ei, sink);
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
