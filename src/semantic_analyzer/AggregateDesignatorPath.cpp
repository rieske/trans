#include "AggregateDesignatorPath.h"

#include <functional>

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
            if (s.indexExpression->evaluateConstant(v)) {
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
        const std::vector<ast::DesignatorStep>& steps, type::Type& outType, int& outOffset,
        std::vector<DesignatorPathItem>& path, int& firstTopLevelIndex, std::string& error) {
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
            std::vector<DesignatorPathItem> foundPath;
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
                    DesignatorPathItem item;
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
            DesignatorPathItem item;
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

} // namespace semantic_analyzer
