#include "SemanticAnalysisVisitorInternal.h"

#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ast/InitializerListExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

namespace {

// Path from root aggregate to a designated (or resume) subobject.
struct PathItem {
    bool isArray { false };
    int index { 0 };
};

// Resolve designator steps to a place and a path from root for resume after fill.
bool resolveDesignator(const type::Type& destType, int baseOffset,
        const std::vector<ast::DesignatorStep>& steps, type::Type& outType, int& outOffset,
        std::vector<PathItem>& path, int& firstTopLevelIndex, std::string& error) {
    if (steps.empty()) {
        error = "empty designator";
        return false;
    }
    type::Type cur = destType;
    int offset = baseOffset;
    path.clear();
    firstTopLevelIndex = -1;

    for (std::size_t si = 0; si < steps.size(); ++si) {
        const auto& step = steps[si];
        if (step.kind == ast::DesignatorStep::Kind::Member) {
            if (!cur.isRecord()) {
                error = "designated initializer member not found";
                return false;
            }
            // DFS: record path indexes (including empty-name anonymous parents).
            std::vector<PathItem> foundPath;
            type::Type foundType = type::voidType();
            int foundOff = 0;
            std::function<bool(const type::Type&, int)> dfs;
            dfs = [&](const type::Type& rec, int base) -> bool {
                for (int i = 0; i < rec.memberCount(); ++i) {
                    std::string n;
                    type::Type t = type::voidType();
                    int o = 0;
                    if (!rec.memberAt(i, n, t, o)) {
                        break;
                    }
                    PathItem item;
                    item.isArray = false;
                    item.index = i;
                    foundPath.push_back(item);
                    if (!n.empty() && n == step.memberName) {
                        foundType = t;
                        foundOff = base + o;
                        return true;
                    }
                    if (n.empty() && t.isRecord()) {
                        if (dfs(t, base + o)) {
                            return true;
                        }
                    }
                    foundPath.pop_back();
                }
                return false;
            };
            if (!dfs(cur, offset)) {
                error = "designated initializer member not found";
                return false;
            }
            if (si == 0 && !foundPath.empty()) {
                firstTopLevelIndex = foundPath.front().index;
            }
            for (auto& p : foundPath) {
                path.push_back(p);
            }
            cur = foundType;
            offset = foundOff;
        } else {
            if (!step.index) {
                error = "designated array index is not a constant expression";
                return false;
            }
            if (!cur.isArray()) {
                error = "array designator on non-array type";
                return false;
            }
            const long idx = *step.index;
            const int n = cur.getArraySize();
            if (n <= 0) {
                error = "array brace initializers for incomplete arrays are not implemented";
                return false;
            }
            if (idx < 0 || idx >= n) {
                error = "designated initializer index out of range";
                return false;
            }
            PathItem item;
            item.isArray = true;
            item.index = static_cast<int>(idx);
            path.push_back(item);
            if (si == 0) {
                firstTopLevelIndex = static_cast<int>(idx);
            }
            offset += static_cast<int>(idx) * cur.getElementStride();
            cur = cur.getElementType();
        }
    }
    outType = cur;
    outOffset = offset;
    return true;
}

bool advancePath(std::vector<PathItem>& path, const type::Type& root) {
    if (path.empty()) {
        return false;
    }
    std::vector<type::Type> containers;
    type::Type cur = root;
    for (const auto& p : path) {
        containers.push_back(cur);
        if (p.isArray) {
            cur = cur.getElementType();
        } else {
            std::string n;
            type::Type t = type::voidType();
            int o = 0;
            if (!cur.memberAt(p.index, n, t, o)) {
                path.clear();
                return false;
            }
            cur = t;
        }
    }
    for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
        const type::Type& container = containers[static_cast<std::size_t>(i)];
        if (path[static_cast<std::size_t>(i)].isArray) {
            const int n = container.getArraySize();
            if (path[static_cast<std::size_t>(i)].index + 1 < n) {
                path[static_cast<std::size_t>(i)].index += 1;
                path.resize(static_cast<std::size_t>(i) + 1);
                return true;
            }
        } else if (container.isUnion()) {
            // Only one active arm; do not resume into sibling union members.
        } else if (path[static_cast<std::size_t>(i)].index + 1 < container.memberCount()) {
            path[static_cast<std::size_t>(i)].index += 1;
            path.resize(static_cast<std::size_t>(i) + 1);
            return true;
        }
    }
    path.clear();
    return false;
}

// Policy sink for one placement pass over an aggregate initializer.
struct AggregateInitSink {
    virtual ~AggregateInitSink() = default;
    virtual void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) = 0;
    virtual void onUnwritten(int offsetBytes, const type::Type& t) = 0;
    virtual void error(const std::string& message) = 0;
    virtual bool ok() const = 0;
};

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink);

void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value, AggregateInitSink& sink);

// Fill aggregate from flat element stream (C current-object). Returns new element index.
// absorbDesignatedAt: element index that is a designator value for this fill and must be
// consumed once (designator stops are otherwise treated as stream barriers).
std::size_t fillFromStream(const type::Type& destType, int baseOffset,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink,
        std::optional<std::size_t> absorbDesignatedAt = std::nullopt);

// Fill root from path (inclusive) through remaining siblings; stop at designators.
std::size_t fillFromPath(const type::Type& root, int baseOffset, std::vector<PathItem> path,
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
        const PathItem& item = path[depth];
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
                    // Stop without bulk-zero: siblings may already hold values from
                    // earlier positionals; outer written[] + end-of-walk zeros true holes.
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
            // Dive into element, then finish later elements of this array.
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
        // Structure / union members. Unions have one active arm - never resume into siblings
        // (mirror advancePath); leftover elements belong to the enclosing aggregate or excess.
        const int nMembers = container.memberCount();
        const bool isUnion = container.isUnion();
        if (depth + 1 == path.size()) {
            const int lastMi = isUnion ? item.index : (nMembers - 1);
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
        if (isUnion) {
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
            // Current-object: non-brace scalar dives into the first subobject only.
            // Remaining subobjects are left for a subsequent stream fill (or onUnwritten).
            if (placeType.isUnion()) {
                if (placeType.memberCount() < 1) {
                    return;
                }
                std::string name;
                type::Type first = type::voidType();
                int off = 0;
                if (placeType.memberAt(0, name, first, off)) {
                    placeAt(first, offsetBytes + off, value, sink);
                }
                return;
            }
            if (placeType.isStructure()) {
                if (placeType.memberCount() < 1) {
                    return;
                }
                std::string name;
                type::Type first = type::voidType();
                int off = 0;
                if (placeType.memberAt(0, name, first, off)) {
                    placeAt(first, offsetBytes + off, value, sink);
                }
                return;
            }
            if (placeType.isArray()) {
                if (placeType.getArraySize() <= 0) {
                    sink.error("array brace initializers for incomplete arrays are not implemented");
                    return;
                }
                placeAt(placeType.getElementType(), offsetBytes, value, sink);
                return;
            }
        }
        sink.onUnwritten(offsetBytes, placeType);
        return;
    }
    // Scalar (possibly braced { e }).
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

std::size_t fillFromStream(const type::Type& destType, int baseOffset,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink,
        std::optional<std::size_t> absorbDesignatedAt) {
    if (!sink.ok()) {
        return ei;
    }
    auto isBarrier = [&](std::size_t i) {
        if (i >= elements.size() || !elements[i].value) {
            return true;
        }
        if (!elements[i].isDesignated()) {
            return false;
        }
        // Allow the designated value that opens this fill to be consumed once.
        return !(absorbDesignatedAt && i == *absorbDesignatedAt);
    };
    auto consumeAbsorb = [&](std::size_t i) {
        if (absorbDesignatedAt && i == *absorbDesignatedAt) {
            absorbDesignatedAt = std::nullopt;
        }
    };

    if (destType.isUnion()) {
        if (isBarrier(ei)) {
            sink.onUnwritten(baseOffset, destType);
            return ei;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
        if (nested) {
            walkAggregateInit(destType, nested, baseOffset, sink);
            consumeAbsorb(ei);
            return ei + 1;
        }
        if (destType.memberCount() < 1) {
            consumeAbsorb(ei);
            return ei + 1;
        }
        std::string name;
        type::Type first = type::voidType();
        int off = 0;
        if (!destType.memberAt(0, name, first, off)) {
            consumeAbsorb(ei);
            return ei + 1;
        }
        if ((first.isStructure() || first.isArray() || first.isUnion()) && !nested) {
            return fillFromStream(first, baseOffset + off, elements, ei, sink, absorbDesignatedAt);
        }
        placeAt(first, baseOffset + off, elements[ei].value.get(), sink);
        consumeAbsorb(ei);
        return ei + 1;
    }
    if (destType.isStructure()) {
        // Brace list is a complete initializer for this structure (same as union).
        if (!isBarrier(ei)) {
            auto* whole = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (whole) {
                walkAggregateInit(destType, whole, baseOffset, sink);
                consumeAbsorb(ei);
                return ei + 1;
            }
        }
        for (int mi = 0; mi < destType.memberCount(); ++mi) {
            std::string name;
            type::Type memberType = type::voidType();
            int offset = 0;
            if (!destType.memberAt(mi, name, memberType, offset)) {
                break;
            }
            if (isBarrier(ei)) {
                sink.onUnwritten(baseOffset + offset, memberType);
                continue;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (nested && (memberType.isStructure() || memberType.isArray() || memberType.isUnion())) {
                walkAggregateInit(memberType, nested, baseOffset + offset, sink);
                consumeAbsorb(ei);
                ++ei;
                continue;
            }
            if ((memberType.isStructure() || memberType.isArray() || memberType.isUnion()) && !nested) {
                ei = fillFromStream(memberType, baseOffset + offset, elements, ei, sink, absorbDesignatedAt);
                continue;
            }
            placeAt(memberType, baseOffset + offset, elements[ei].value.get(), sink);
            consumeAbsorb(ei);
            ++ei;
        }
        return ei;
    }
    if (destType.isArray()) {
        const int n = destType.getArraySize();
        if (n <= 0) {
            sink.error("array brace initializers for incomplete arrays are not implemented");
            return ei;
        }
        // Brace list is a complete initializer for this array (same as union/struct).
        if (!isBarrier(ei)) {
            auto* whole = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (whole) {
                walkAggregateInit(destType, whole, baseOffset, sink);
                consumeAbsorb(ei);
                return ei + 1;
            }
        }
        const int stride = destType.getElementStride();
        const type::Type elem = destType.getElementType();
        for (int i = 0; i < n; ++i) {
            if (isBarrier(ei)) {
                sink.onUnwritten(baseOffset + i * stride, elem);
                continue;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (nested && (elem.isStructure() || elem.isArray() || elem.isUnion())) {
                walkAggregateInit(elem, nested, baseOffset + i * stride, sink);
                consumeAbsorb(ei);
                ++ei;
                continue;
            }
            if ((elem.isStructure() || elem.isArray() || elem.isUnion()) && !nested) {
                ei = fillFromStream(elem, baseOffset + i * stride, elements, ei, sink, absorbDesignatedAt);
                continue;
            }
            placeAt(elem, baseOffset + i * stride, elements[ei].value.get(), sink);
            consumeAbsorb(ei);
            ++ei;
        }
        return ei;
    }
    if (isBarrier(ei)) {
        sink.onUnwritten(baseOffset, destType);
        return ei;
    }
    placeAt(destType, baseOffset, elements[ei].value.get(), sink);
    consumeAbsorb(ei);
    return ei + 1;
}

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink) {
    if (!sink.ok() || !list) {
        return;
    }
    const auto& src = list->getElements();

    auto foldSteps = [&](const ast::InitializerElement& el, std::vector<ast::DesignatorStep>& stepsOut) -> bool {
        stepsOut.clear();
        for (const auto& s : el.designator) {
            ast::DesignatorStep copy;
            copy.kind = s.kind;
            copy.memberName = s.memberName;
            copy.index = s.index;
            if (!copy.index && s.indexExpression) {
                long v = 0;
                if (s.indexExpression->evaluateConstant(v)) {
                    copy.index = v;
                } else {
                    sink.error("designated array index is not a constant expression");
                    return false;
                }
            } else if (copy.kind == ast::DesignatorStep::Kind::Index && !copy.index) {
                sink.error("designated array index is not a constant expression");
                return false;
            }
            stepsOut.push_back(std::move(copy));
        }
        return true;
    };

    auto applyDesignator = [&](std::size_t& ei, std::vector<bool>* written, int nSlots, bool isArrayRoot) {
        const auto& el = src[ei];
        std::vector<ast::DesignatorStep> steps;
        if (!foldSteps(el, steps)) {
            return;
        }
        type::Type placeType = type::voidType();
        int placeOff = 0;
        std::vector<PathItem> path;
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
        // Zero whole first-level slot once before nested leaf stores.
        if (firstIdx >= 0 && path.size() > 1 && written && !(*written)[static_cast<std::size_t>(firstIdx)]) {
            if (isArrayRoot) {
                const int stride = targetType.getElementStride();
                sink.onUnwritten(baseOffset + firstIdx * stride, targetType.getElementType());
            } else {
                std::string name;
                type::Type mt = type::voidType();
                int off = 0;
                if (targetType.memberAt(firstIdx, name, mt, off)) {
                    sink.onUnwritten(baseOffset + off, mt);
                }
            }
        }
        mark(firstIdx);
        // Fill designated object from this element onward (current-object inside D).
        // Absorb the leading designator element so it is not treated as a stream barrier.
        const std::size_t designatorEi = ei;
        ei = fillFromStream(placeType, placeOff, src, ei, sink, designatorEi);
        // Resume after D: advance path past designated object, fill remainder of root.
        if (advancePath(path, targetType)) {
            ei = fillFromPath(targetType, baseOffset, path, src, ei, sink, mark);
        }
    };

    if (targetType.isUnion()) {
        if (src.empty() || !src.front().value) {
            sink.onUnwritten(baseOffset, targetType);
            return;
        }
        // C: whole union is zeroed, then list elements apply left-to-right (last wins).
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
            // At most one non-designated element, and only before any designator.
            if (sawPositional || sawDesignator) {
                sink.error("excess elements in union initializer");
                ++ei;
                continue;
            }
            sawPositional = true;
            if (targetType.memberCount() < 1) {
                ++ei;
                continue;
            }
            std::string name;
            type::Type first = type::voidType();
            int off = 0;
            if (!targetType.memberAt(0, name, first, off)) {
                ++ei;
                continue;
            }
            placeAt(first, baseOffset + off, el.value.get(), sink);
            ++ei;
        }
        return;
    }

    if (targetType.isStructure()) {
        const int nMembers = targetType.memberCount();
        std::vector<bool> written(static_cast<std::size_t>(nMembers), false);
        std::size_t ei = 0;
        int positional = 0;

        auto markWritten = [&](int mi) {
            if (mi >= 0 && mi < nMembers) {
                written[static_cast<std::size_t>(mi)] = true;
            }
        };

        while (ei < src.size() && sink.ok()) {
            const auto& el = src[ei];
            if (!el.value) {
                ++ei;
                continue;
            }
            if (el.isDesignated()) {
                applyDesignator(ei, &written, nMembers, false);
                // Current-object positionals after this designator were already
                // consumed inside applyDesignator (fillFromStream + fillFromPath).
                // Do not reopen earlier unwritten holes; leftovers are excess.
                // A later designator re-enters via applyDesignator.
                positional = nMembers;
                continue;
            }
            if (positional >= nMembers) {
                sink.error("excess elements in structure initializer");
                ++ei;
                continue;
            }
            std::string name;
            type::Type memberType = type::voidType();
            int offset = 0;
            if (!targetType.memberAt(positional, name, memberType, offset)) {
                break;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
            if (nested && (memberType.isStructure() || memberType.isArray() || memberType.isUnion())) {
                markWritten(positional);
                walkAggregateInit(memberType, nested, baseOffset + offset, sink);
                ++ei;
                ++positional;
                continue;
            }
            if ((memberType.isStructure() || memberType.isArray() || memberType.isUnion()) && !nested) {
                const std::size_t before = ei;
                ei = fillFromStream(memberType, baseOffset + offset, src, ei, sink);
                if (ei == before) {
                    sink.onUnwritten(baseOffset + offset, memberType);
                }
                markWritten(positional);
                ++positional;
                continue;
            }
            markWritten(positional);
            placeAt(memberType, baseOffset + offset, el.value.get(), sink);
            ++ei;
            ++positional;
        }
        for (int i = 0; i < nMembers && sink.ok(); ++i) {
            if (!written[static_cast<std::size_t>(i)]) {
                std::string name;
                type::Type memberType = type::voidType();
                int offset = 0;
                if (targetType.memberAt(i, name, memberType, offset)) {
                    sink.onUnwritten(baseOffset + offset, memberType);
                }
            }
        }
        return;
    }

    if (targetType.isArray()) {
        const int n = targetType.getArraySize();
        if (n <= 0) {
            sink.error("array brace initializers for incomplete arrays are not implemented");
            return;
        }
        std::vector<bool> written(static_cast<std::size_t>(n), false);
        std::size_t ei = 0;
        int positional = 0;
        const int stride = targetType.getElementStride();
        const type::Type elem = targetType.getElementType();

        while (ei < src.size() && sink.ok()) {
            const auto& el = src[ei];
            if (!el.value) {
                ++ei;
                continue;
            }
            if (el.isDesignated()) {
                applyDesignator(ei, &written, n, true);
                // Same as struct: path fill already took successive elements;
                // leftovers must not land in earlier array holes.
                positional = n;
                continue;
            }
            if (positional >= n) {
                sink.error("excess elements in array initializer");
                ++ei;
                continue;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
            if (nested && (elem.isStructure() || elem.isArray() || elem.isUnion())) {
                written[static_cast<std::size_t>(positional)] = true;
                walkAggregateInit(elem, nested, baseOffset + positional * stride, sink);
                ++ei;
                ++positional;
                continue;
            }
            if ((elem.isStructure() || elem.isArray() || elem.isUnion()) && !nested) {
                const std::size_t before = ei;
                ei = fillFromStream(elem, baseOffset + positional * stride, src, ei, sink);
                if (ei == before) {
                    sink.onUnwritten(baseOffset + positional * stride, elem);
                }
                written[static_cast<std::size_t>(positional)] = true;
                ++positional;
                continue;
            }
            written[static_cast<std::size_t>(positional)] = true;
            placeAt(elem, baseOffset + positional * stride, el.value.get(), sink);
            ++ei;
            ++positional;
        }
        for (int i = 0; i < n && sink.ok(); ++i) {
            if (!written[static_cast<std::size_t>(i)]) {
                sink.onUnwritten(baseOffset + i * stride, elem);
            }
        }
        return;
    }
    sink.error("brace initializer for non-aggregate type");
}

// --- Local field-init sink ---

struct FieldPlanSink : AggregateInitSink {
    SemanticAnalysisVisitor& visitor;
    semantic_analyzer::SymbolTable& symbolTable;
    symbols::AnnotationStore& annotations;
    translation_unit::Context context;
    std::vector<symbols::StructFieldInit>& plan;
    bool failed { false };

    FieldPlanSink(SemanticAnalysisVisitor& v, semantic_analyzer::SymbolTable& st,
            symbols::AnnotationStore& ann, translation_unit::Context ctx,
            std::vector<symbols::StructFieldInit>& p)
            : visitor { v }, symbolTable { st }, annotations { ann }, context { std::move(ctx) },
              plan { p } {
    }

    bool ok() const override { return !failed; }

    void error(const std::string& message) override {
        failed = true;
        visitor.semanticError(message, context);
    }

    void emitZero(int offsetBytes, const type::Type& storeType) {
        symbols::StructFieldInit field;
        field.offsetBytes = offsetBytes;
        auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
        field.addressName = addr.getName();
        auto zero = symbolTable.createTemporarySymbol(storeType);
        field.zeroInitialize = true;
        field.sourceName = zero.getName();
        plan.push_back(std::move(field));
    }

    void zeroRegion(int offsetBytes, const type::Type& t) {
        if (t.isUnion()) {
            // Codegen stores 1/4/8 only; walk size like DataWordSink so large unions
            // are fully zeroed (one emitZero of a multi-word type is truncated).
            const int size = t.getSize();
            int off = 0;
            while (off + type::object_abi::MACHINE_WORD_SIZE <= size) {
                emitZero(offsetBytes + off, type::signedLong());
                off += type::object_abi::MACHINE_WORD_SIZE;
            }
            if (off + 4 <= size) {
                emitZero(offsetBytes + off, type::signedInteger());
                off += 4;
            }
            while (off < size) {
                emitZero(offsetBytes + off, type::signedCharacter());
                ++off;
            }
            return;
        }
        if (t.isStructure()) {
            for (int i = 0; i < t.memberCount(); ++i) {
                std::string name;
                type::Type mt = type::voidType();
                int off = 0;
                if (!t.memberAt(i, name, mt, off)) {
                    break;
                }
                zeroRegion(offsetBytes + off, mt);
            }
            return;
        }
        if (t.isArray()) {
            const int n = t.getArraySize();
            if (n <= 0) {
                error("array brace initializers for incomplete arrays are not implemented");
                return;
            }
            const int stride = t.getElementStride();
            const type::Type elem = t.getElementType();
            for (int i = 0; i < n; ++i) {
                zeroRegion(offsetBytes + i * stride, elem);
            }
            return;
        }
        emitZero(offsetBytes, t);
    }

    void onUnwritten(int offsetBytes, const type::Type& t) override {
        zeroRegion(offsetBytes, t);
    }

    void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) override {
        symbols::StructFieldInit field;
        field.offsetBytes = offsetBytes;
        auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
        field.addressName = addr.getName();
        if (value && value->hasResultSymbol(annotations)) {
            const type::Type src = assignSourceType(*value, storeType, annotations);
            if (!storeType.canAssignFrom(src)) {
                failed = true;
            }
            visitor.typeCheck(src, storeType, context);
            field.zeroInitialize = false;
            field.sourceName = value->getResultSymbol(annotations)->getName();
        } else {
            auto zero = symbolTable.createTemporarySymbol(storeType);
            field.zeroInitialize = true;
            field.sourceName = zero.getName();
        }
        // Still record the slot when typeCheck fails so error recovery stays simple;
        // lowerLocalInitializer drops the plan when !sink.ok().
        plan.push_back(std::move(field));
    }
};

// --- Global constant data-word sink ---

struct DataWordSink : AggregateInitSink {
    SemanticAnalysisVisitor& visitor;
    translation_unit::Context context;
    std::vector<std::string>& words;
    int wordCount;
    bool failed { false };

    DataWordSink(SemanticAnalysisVisitor& v, translation_unit::Context ctx,
            std::vector<std::string>& w, int wc)
            : visitor { v }, context { std::move(ctx) }, words { w }, wordCount { wc } {
    }

    bool ok() const override { return !failed; }

    void error(const std::string& message) override {
        failed = true;
        visitor.semanticError(message, context);
    }

    static std::string formatWord(unsigned long long v) {
        if (v > 0x7fffffffull) {
            std::ostringstream hex;
            hex << "0x" << std::hex << v;
            return hex.str();
        }
        return std::to_string(v);
    }

    static bool parseWord(const std::string& s, unsigned long long& out) {
        if (s.empty()) {
            return false;
        }
        try {
            std::size_t idx = 0;
            if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
                out = std::stoull(s, &idx, 16);
            } else {
                long long signedVal = std::stoll(s, &idx, 10);
                out = static_cast<unsigned long long>(signedVal);
            }
            return idx == s.size();
        } catch (...) {
            return false;
        }
    }

    void storeAt(int offsetBytes, long value, int storeSizeBytes) {
        if (offsetBytes < 0 || storeSizeBytes <= 0) {
            return;
        }
        const int wi = type::object_abi::wordIndexAt(offsetBytes);
        if (wi < 0 || wi >= wordCount) {
            return;
        }
        if (storeSizeBytes >= type::object_abi::MACHINE_WORD_SIZE) {
            words[static_cast<std::size_t>(wi)] = formatWord(static_cast<unsigned long long>(value));
            return;
        }
        unsigned long long wordVal = 0;
        parseWord(words[static_cast<std::size_t>(wi)], wordVal);
        const int lane = offsetBytes % type::object_abi::MACHINE_WORD_SIZE;
        const int bits = storeSizeBytes * 8;
        const unsigned long long mask = bits >= 64 ? ~0ull : ((1ull << bits) - 1ull);
        wordVal &= ~(mask << (lane * 8));
        wordVal |= (static_cast<unsigned long long>(value) & mask) << (lane * 8);
        words[static_cast<std::size_t>(wi)] = formatWord(wordVal);
    }

    // Clear previously written lanes when a later init re-zeros a region (not only
    // never-touched holes - prefill alone is not enough after partial stores).
    void zeroRegion(int offsetBytes, const type::Type& t) {
        if (t.isUnion()) {
            const int size = t.getSize();
            int off = 0;
            while (off + type::object_abi::MACHINE_WORD_SIZE <= size) {
                storeAt(offsetBytes + off, 0, type::object_abi::MACHINE_WORD_SIZE);
                off += type::object_abi::MACHINE_WORD_SIZE;
            }
            while (off < size) {
                storeAt(offsetBytes + off, 0, 1);
                ++off;
            }
            return;
        }
        if (t.isStructure()) {
            for (int i = 0; i < t.memberCount(); ++i) {
                std::string name;
                type::Type mt = type::voidType();
                int off = 0;
                if (!t.memberAt(i, name, mt, off)) {
                    break;
                }
                zeroRegion(offsetBytes + off, mt);
            }
            return;
        }
        if (t.isArray()) {
            const int n = t.getArraySize();
            if (n <= 0) {
                error("array brace initializers for incomplete arrays are not implemented");
                return;
            }
            const int stride = t.getElementStride();
            const type::Type elem = t.getElementType();
            for (int i = 0; i < n; ++i) {
                zeroRegion(offsetBytes + i * stride, elem);
            }
            return;
        }
        storeAt(offsetBytes, 0, t.getSize());
    }

    void onUnwritten(int offsetBytes, const type::Type& t) override {
        zeroRegion(offsetBytes, t);
    }

    void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) override {
        if (!value) {
            return;
        }
        long v = 0;
        if (!value->evaluateConstant(v)) {
            error("global brace initializer is not a constant expression");
            return;
        }
        storeAt(offsetBytes, v, storeType.getSize());
    }
};

} // namespace

void SemanticAnalysisVisitor::lowerLocalInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& objectType) {
    if (!declarator.hasInitializer()) {
        return;
    }

    if (symbolTable.isAtFileScope()) {
        long initValue = 0;
        if (declarator.getInitializer()->evaluateConstant(initValue)) {
            symbolTable.setGlobalInitializer(declarator.getName(), initValue);
            return;
        }
        if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
            if (!(objectType.isRecord() || objectType.isArray())) {
                if (list->getElements().size() == 1 && list->getElements().front().value
                        && list->getElements().front().value->evaluateConstant(initValue)) {
                    symbolTable.setGlobalInitializer(declarator.getName(), initValue);
                    return;
                }
                semanticError("global brace initializer is not a constant expression", declarator.getContext());
                return;
            }
            const int wordCount = type::object_abi::dataWords(objectType.getSize());
            if (wordCount <= 0) {
                return;
            }
            std::vector<std::string> words(static_cast<std::size_t>(wordCount), "0");
            DataWordSink sink { *this, declarator.getContext(), words, wordCount };
            walkAggregateInit(objectType, list, 0, sink);
            if (!sink.ok()) {
                return;
            }
            symbolTable.setGlobalMultiWordInitializer(declarator.getName(), std::move(words));
            return;
        }
        semanticError("global initializer is not a constant expression", declarator.getContext());
        return;
    }

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isRecord() || objectType.isArray()) {
            std::vector<symbols::StructFieldInit> plan;
            FieldPlanSink sink { *this, symbolTable, annotations(), declarator.getContext(), plan };
            walkAggregateInit(objectType, list, 0, sink);
            if (sink.ok()) {
                annotations().setStructFieldInits(&declarator, std::move(plan));
            }
            return;
        }
        // Top-level scalar braces.
        if (list->getElements().size() > 1) {
            semanticError("excess elements in scalar initializer", declarator.getContext());
            return;
        }
        if (list->getElements().empty() || !list->getElements().front().value) {
            return;
        }
        ast::Expression* value = list->getElements().front().value.get();
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
        while (nested) {
            if (nested->getElements().size() > 1) {
                semanticError("excess elements in scalar initializer", declarator.getContext());
                return;
            }
            if (nested->getElements().empty() || !nested->getElements().front().value) {
                return;
            }
            value = nested->getElements().front().value.get();
            nested = dynamic_cast<ast::InitializerListExpression*>(value);
        }
        if (value && value->hasResultSymbol(annotations())) {
            type::Type src = assignSourceType(*value, objectType, annotations());
            if (!value->holdsAggregateAddress() || objectType.isPointer()) {
                typeCheck(src, objectType, declarator.getContext());
            }
            list->setResultSymbol(annotations(), *value->getResultSymbol(annotations()));
        }
        return;
    }

    ast::Expression* initExpr = declarator.getInitializer();
    if (initExpr && initExpr->hasResultSymbol(annotations())) {
        type::Type src = assignSourceType(*initExpr, objectType, annotations());
        if (!initExpr->holdsAggregateAddress() || objectType.isPointer()) {
            typeCheck(src, objectType, declarator.getContext());
        }
    }
}

} // namespace semantic_analyzer
