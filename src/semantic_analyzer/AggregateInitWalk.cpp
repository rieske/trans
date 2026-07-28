#include "AggregateInitWalk.h"

#include "AggregateDesignatorPath.h"

#include "ast/InitializerListExpression.h"

#include <functional>
#include <vector>

namespace semantic_analyzer {
namespace {

bool firstSubobjectOf(const type::Type& t, int baseOffset, type::Type& outType, int& outAbsOffset,
        AggregateInitSink& sink) {
    if (t.isRecord()) {
        if (t.memberCount() < 1) {
            return false;
        }
        std::string name;
        int rel = 0;
        if (!t.memberAt(0, name, outType, rel)) {
            return false;
        }
        outAbsOffset = baseOffset + rel;
        return true;
    }
    if (t.isArray()) {
        if (t.getArraySize() <= 0) {
            sink.error("array brace initializers for incomplete arrays are not implemented");
            return false;
        }
        outType = t.getElementType();
        outAbsOffset = baseOffset;
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

void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value, AggregateInitSink& sink);

std::size_t fillFromStream(const type::Type& destType, int baseOffset,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink);

std::size_t fillFromPath(const type::Type& root, int baseOffset, std::vector<DesignatorPathItem> path,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink,
        const std::function<void(int)>& markTopLevel);

void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value, AggregateInitSink& sink) {
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
        if (value) {
            type::Type first = type::voidType();
            int off = 0;
            if (!firstSubobjectOf(placeType, offsetBytes, first, off, sink)) {
                return;
            }
            placeAt(first, off, value, sink);
            return;
        }
        sink.onUnwritten(offsetBytes, placeType);
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

std::size_t fillSlottedFromStream(const AggregateSlots& slots,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink) {
    const int n = slots.count();
    if (slots.isArray && n <= 0) {
        sink.error("array brace initializers for incomplete arrays are not implemented");
        return ei;
    }
    for (int i = 0; i < n && sink.ok(); ++i) {
        type::Type slotType = type::voidType();
        int slotOff = 0;
        if (!slots.slotAt(i, slotType, slotOff)) {
            break;
        }
        if (ei >= elements.size() || !elements[ei].value || elements[ei].isDesignated()) {
            sink.onUnwritten(slotOff, slotType);
            continue;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
        if (nested && slotType.isAggregate()) {
            walkAggregateInit(slotType, nested, slotOff, sink);
            ++ei;
            continue;
        }
        if (slotType.isAggregate() && !nested) {
            ei = fillFromStream(slotType, slotOff, elements, ei, sink);
            continue;
        }
        placeAt(slotType, slotOff, elements[ei].value.get(), sink);
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
            sink.onUnwritten(baseOffset, destType);
            return ei;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
        if (nested) {
            walkAggregateInit(destType, nested, baseOffset, sink);
            return ei + 1;
        }
        sink.onUnwritten(baseOffset, destType);
        type::Type first = type::voidType();
        int off = 0;
        if (!firstSubobjectOf(destType, baseOffset, first, off, sink)) {
            return ei + 1;
        }
        if (first.isAggregate()) {
            return fillFromStream(first, off, elements, ei, sink);
        }
        placeAt(first, off, elements[ei].value.get(), sink);
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
        sink.onUnwritten(baseOffset, destType);
        return ei;
    }
    placeAt(destType, baseOffset, elements[ei].value.get(), sink);
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
                        sink.onUnwritten(containerOff + i * stride, elem);
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
        if (depth + 1 == path.size()) {
            const int lastMi = noSiblingResume ? item.index : (nMembers - 1);
            for (int mi = item.index; mi <= lastMi && mi < nMembers && sink.ok(); ++mi) {
                std::string name;
                type::Type memberType = type::voidType();
                int moff = 0;
                if (!container.memberAt(mi, name, memberType, moff)) {
                    break;
                }
                if (ei >= elements.size() || elements[ei].isDesignated()) {
                    return ei;
                }
                if (topLevelHint >= 0 && depth == 0) {
                    markTopLevel(mi);
                }
                ei = fillFromStream(memberType, containerOff + moff, elements, ei, sink);
            }
            return ei;
        }
        std::string name;
        type::Type memberType = type::voidType();
        int moff = 0;
        if (!container.memberAt(item.index, name, memberType, moff)) {
            return ei;
        }
        if (topLevelHint >= 0 && depth == 0) {
            markTopLevel(item.index);
        }
        ei = rec(memberType, containerOff + moff, depth + 1, -1);
        if (noSiblingResume) {
            return ei;
        }
        for (int mi = item.index + 1; mi < nMembers && sink.ok(); ++mi) {
            std::string n2;
            type::Type t2 = type::voidType();
            int o2 = 0;
            if (!container.memberAt(mi, n2, t2, o2)) {
                break;
            }
            if (ei >= elements.size() || elements[ei].isDesignated()) {
                return ei;
            }
            if (topLevelHint >= 0 && depth == 0) {
                markTopLevel(mi);
            }
            ei = fillFromStream(t2, containerOff + o2, elements, ei, sink);
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
        type::Type slotType = type::voidType();
        int slotOff = 0;
        if (!slots.slotAt(positional, slotType, slotOff)) {
            break;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
        if (nested && slotType.isAggregate()) {
            written[static_cast<std::size_t>(positional)] = true;
            walkAggregateInit(slotType, nested, slotOff, sink);
            ++ei;
            ++positional;
            continue;
        }
        if (slotType.isAggregate() && !nested) {
            const std::size_t before = ei;
            ei = fillFromStream(slotType, slotOff, src, ei, sink);
            if (ei == before) {
                sink.onUnwritten(slotOff, slotType);
            }
            written[static_cast<std::size_t>(positional)] = true;
            ++positional;
            continue;
        }
        written[static_cast<std::size_t>(positional)] = true;
        placeAt(slotType, slotOff, el.value.get(), sink);
        ++ei;
        ++positional;
    }
    for (int i = 0; i < n && sink.ok(); ++i) {
        if (!written[static_cast<std::size_t>(i)]) {
            type::Type slotType = type::voidType();
            int slotOff = 0;
            if (slots.slotAt(i, slotType, slotOff)) {
                sink.onUnwritten(slotOff, slotType);
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
        auto mark = [&](int idx) {
            if (written && idx >= 0 && idx < nSlots) {
                (*written)[static_cast<std::size_t>(idx)] = true;
            }
        };
        if (firstIdx >= 0 && path.size() > 1 && written && !(*written)[static_cast<std::size_t>(firstIdx)]) {
            if (isArrayRoot) {
                const int stride = targetType.getElementStride();
                sink.onUnwritten(baseOffset + firstIdx * stride, targetType.getElementType());
            } else {
                type::Type mt = type::voidType();
                int off = 0;
                std::string name;
                if (targetType.memberAt(firstIdx, name, mt, off)) {
                    sink.onUnwritten(baseOffset + off, mt);
                }
            }
        }
        mark(firstIdx);

        auto* nestedValue = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
        if (nestedValue) {
            walkAggregateInit(placeType, nestedValue, placeOff, sink);
            ++ei;
        } else if (placeType.isAggregate()) {
            sink.onUnwritten(placeOff, placeType);
            placeAt(placeType, placeOff, el.value.get(), sink);
            ++ei;
        } else {
            placeAt(placeType, placeOff, el.value.get(), sink);
            ++ei;
        }
        if (advanceDesignatorPath(path, targetType)) {
            ei = fillFromPath(targetType, baseOffset, path, src, ei, sink, mark);
        }
    };

    if (targetType.isUnion()) {
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
                applyDesignator(ei, nullptr, 0, false);
                continue;
            }
            if (sawPositional || sawDesignator) {
                sink.error("excess elements in union initializer");
                ++ei;
                continue;
            }
            sawPositional = true;
            type::Type first = type::voidType();
            int off = 0;
            if (!firstSubobjectOf(targetType, baseOffset, first, off, sink)) {
                ++ei;
                continue;
            }
            const std::size_t before = ei;
            ei = fillFromStream(first, off, src, ei, sink);
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
