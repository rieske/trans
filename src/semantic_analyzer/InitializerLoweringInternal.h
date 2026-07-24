#ifndef SEMANTIC_INITIALIZERLOWERINGINTERNAL_H_
#define SEMANTIC_INITIALIZERLOWERINGINTERNAL_H_

// Shared placement navigation + sink interface for brace/designated initializers.
// lowerToDataWords and lowerToFieldInits supply sinks; walkAggregateInit owns the walk.

#include <cstddef>
#include <string>
#include <vector>

#include "ast/InitializerListExpression.h"
#include "types/Type.h"
#include "Symbols.h"

namespace semantic_analyzer {


class ValueEntry;

namespace init_detail {

inline bool isCharArrayType(const type::Type& arrayType) {
    if (!arrayType.isArray()) {
        return false;
    }
    const type::Type elem = arrayType.getElementType();
    return elem.isPrimitive() && elem.getSize() == 1;
}

inline bool findMemberIndex(const std::vector<type::Type::Member>& members,
        const std::string& name, std::size_t& outIndex) {
    for (std::size_t mi = 0; mi < members.size(); ++mi) {
        if (members[mi].name == name) {
            outIndex = mi;
            return true;
        }
    }
    return false;
}

inline bool resolveMemberPath(const std::vector<type::Type::Member>& members,
        std::size_t firstIndex,
        const std::vector<std::string>& path,
        int baseOffset,
        const type::Type*& outType,
        int& outOffset) {
    if (firstIndex >= members.size() || path.empty() || !members[firstIndex].type) {
        return false;
    }
    outType = members[firstIndex].type.get();
    outOffset = baseOffset + members[firstIndex].offsetBytes;
    for (std::size_t pi = 1; pi < path.size(); ++pi) {
        if (!outType->isRecord()) {
            return false;
        }
        bool found = false;
        for (const auto& nm : outType->getStructMembers()) {
            if (nm.name == path[pi] && nm.type) {
                outOffset += nm.offsetBytes;
                outType = nm.type.get();
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return outType != nullptr;
}

// Policy sink for one placement pass over an aggregate initializer.
// Walker never asks "are you zeroing?"; it reports unwritten slots via onUnwritten.
struct AggregateInitSink {
    virtual ~AggregateInitSink() = default;

    virtual void placeScalar(int offsetBytes, const type::Type& storeType,
            ast::Expression* value) = 0;

    virtual bool placeStringArray(int offsetBytes, const type::Type& arrayType,
            ast::Expression* value) = 0;

    virtual void placeAggregateCopy(int offsetBytes, const type::Type& storeType,
            ValueEntry* source) = 0;

    // Unwritten / defaulted region (fields zero; data no-ops — words prefilled "0").
    virtual void onUnwritten(int offsetBytes, const type::Type& t) = 0;
};

void walkAggregateInit(const type::Type& targetType,
        const ast::InitializerListExpression* list,
        int baseOffset,
        AggregateInitSink& sink);

} // namespace init_detail
} // namespace semantic_analyzer

#endif // SEMANTIC_INITIALIZERLOWERINGINTERNAL_H_
