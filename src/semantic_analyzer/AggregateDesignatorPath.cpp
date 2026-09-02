#include "AggregateDesignatorPath.h"

#include "AggregateInitError.h"

#include <optional>

namespace semantic_analyzer {

bool foldDesignatorSteps(const ast::InitializerElement& el,
        std::vector<ast::DesignatorStep>& stepsOut, std::string& error) {
    stepsOut.clear();
    for (const auto& s : el.designator) {
        ast::DesignatorStep copy;
        copy.kind = s.kind;
        copy.memberName = s.memberName;
        copy.index = s.index;
        if (!copy.index && s.indexExpression) {
            long v = 0;
            if (s.indexExpression->foldToHostLong(v)) {
                copy.index = v;
            } else {
                error = "designated array index is not a constant expression";
                return false;
            }
        } else if (copy.kind == ast::DesignatorStep::Kind::Index && !copy.index) {
            error = "designated array index is not a constant expression";
            return false;
        }
        stepsOut.push_back(std::move(copy));
    }
    return true;
}

bool resolveDesignator(const type::Type& destType, int baseOffset,
        const std::vector<ast::DesignatorStep>& steps, type::FoundMember& outSlot,
        std::vector<DesignatorPathItem>& path, std::string& error) {
    if (steps.empty()) {
        error = "empty designator";
        return false;
    }
    type::Type cur = destType;
    int offset = baseOffset;
    std::optional<type::BitField> bits;
    std::string name;
    path.clear();

    for (const auto& step : steps) {
        if (step.kind == ast::DesignatorStep::Kind::Member) {
            auto found = type::lookupMemberPath(cur, step.memberName);
            if (!found) {
                error = "designated initializer member not found";
                return false;
            }
            for (int index : found->indices) {
                DesignatorPathItem item;
                item.isArray = false;
                item.index = index;
                path.push_back(item);
            }
            cur = found->member.type;
            offset += found->member.offsetBytes;
            bits = found->member.bitField;
            name = found->member.name;
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
                error = unsizedArrayInitError(cur);
                return false;
            }
            if (idx < 0 || idx >= n) {
                error = "designated initializer index out of range";
                return false;
            }
            DesignatorPathItem item;
            item.isArray = true;
            item.index = static_cast<int>(idx);
            path.push_back(item);
            offset += static_cast<int>(idx) * cur.getElementStride();
            cur = cur.getElementType();
            bits.reset();
            name.clear();
        }
    }
    outSlot = type::FoundMember { name, cur, offset, bits };
    return true;
}

bool advanceDesignatorPath(std::vector<DesignatorPathItem>& path, const type::Type& root) {
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
            auto member = type::memberAt(cur, p.index);
            if (!member) {
                path.clear();
                return false;
            }
            cur = member->type;
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

} // namespace semantic_analyzer
