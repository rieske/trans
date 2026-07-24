#include "InitializerLoweringInternal.h"

#include <vector>

#include "ast/Expression.h"
#include "symbols/ValueEntry.h"
#include "Symbols.h"

namespace semantic_analyzer {

namespace init_detail {

namespace {

void walkAggregateInitImpl(const type::Type& targetType,
        const ast::InitializerListExpression* list,
        int baseOffset,
        AggregateInitSink& sink);

void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value,
        AggregateInitSink& sink) {
    if (auto* nestedList = value
            ? dynamic_cast<ast::InitializerListExpression*>(value) : nullptr) {
        if (placeType.isAggregate()) {
            walkAggregateInitImpl(placeType, nestedList, offsetBytes, sink);
            return;
        }
    }
    if (placeType.isArray() && sink.placeStringArray(offsetBytes, placeType, value)) {
        return;
    }
    ValueEntry* source =
            (value && value->hasResultSymbol()) ? value->getResultSymbol() : nullptr;
    if (placeType.isAggregate()) {
        if (source && source->getType().isAggregate()) {
            sink.placeAggregateCopy(offsetBytes, placeType, source);
        } else if (!sink.placeStringArray(offsetBytes, placeType, value)) {
            sink.onUnwritten(offsetBytes, placeType);
        }
        return;
    }
    if (source || value) {
        sink.placeScalar(offsetBytes, placeType, value);
    } else {
        sink.onUnwritten(offsetBytes, placeType);
    }
}

void walkRecord(const type::Type& structType,
        const ast::InitializerListExpression* list,
        int baseOffset,
        AggregateInitSink& sink) {
    const auto& members = structType.getStructMembers();
    std::vector<bool> written(members.size(), false);

    auto applyMember = [&](std::size_t memberIndex, ast::Expression* value) {
        if (memberIndex >= members.size() || !members[memberIndex].type) {
            return;
        }
        written[memberIndex] = true;
        const type::Type& memberType = *members[memberIndex].type;
        const int memberOffset = baseOffset + members[memberIndex].offsetBytes;
        placeAt(memberType, memberOffset, value, sink);
    };

    auto applyPath = [&](std::size_t firstIndex, const std::vector<std::string>& path,
            ast::Expression* value) {
        if (firstIndex >= members.size() || path.empty() || !members[firstIndex].type) {
            return;
        }
        if (!written[firstIndex]) {
            // Prefix member was not positionally initialized; report unwritten whole member
            // before diving into a nested designator (.a.b).
            sink.onUnwritten(baseOffset + members[firstIndex].offsetBytes,
                    *members[firstIndex].type);
            written[firstIndex] = true;
        }
        const type::Type* curType = nullptr;
        int curOffset = 0;
        if (!resolveMemberPath(members, firstIndex, path, baseOffset, curType, curOffset)) {
            return;
        }
        placeAt(*curType, curOffset, value, sink);
    };

    std::size_t positional = 0;
    for (const auto& element : list->getElements()) {
        if (!element.value) {
            continue;
        }
        if (!element.memberPath.empty()) {
            std::size_t mi = 0;
            if (!findMemberIndex(members, element.memberPath[0], mi)) {
                continue;
            }
            if (element.memberPath.size() == 1) {
                applyMember(mi, element.value.get());
            } else {
                applyPath(mi, element.memberPath, element.value.get());
            }
            positional = mi + 1;
        } else if (element.arrayIndex) {
            long idx = *element.arrayIndex;
            if (idx >= 0 && static_cast<std::size_t>(idx) < members.size()) {
                applyMember(static_cast<std::size_t>(idx), element.value.get());
                positional = static_cast<std::size_t>(idx) + 1;
            }
        } else if (positional < members.size()) {
            applyMember(positional, element.value.get());
            positional = positional + 1;
        }
    }

    if (structType.isUnion()) {
        bool anyWritten = false;
        for (bool w : written) {
            if (w) {
                anyWritten = true;
                break;
            }
        }
        if (!anyWritten) {
            sink.onUnwritten(baseOffset, structType);
        }
    } else {
        for (std::size_t mi = 0; mi < members.size(); ++mi) {
            if (!written[mi] && members[mi].type) {
                sink.onUnwritten(baseOffset + members[mi].offsetBytes, *members[mi].type);
            }
        }
    }
}

void walkArray(const type::Type& arrayType,
        const ast::InitializerListExpression* list,
        int baseOffset,
        AggregateInitSink& sink) {
    const int length = arrayType.getArraySize();
    if (length <= 0) {
        return;
    }
    const int stride = arrayType.getElementStride();
    const type::Type elementType = arrayType.getElementType();
    std::vector<bool> written(static_cast<std::size_t>(length), false);

    auto applyElement = [&](std::size_t index, ast::Expression* value) {
        if (index >= static_cast<std::size_t>(length)) {
            return;
        }
        written[index] = true;
        const int offset = baseOffset + static_cast<int>(index) * stride;
        placeAt(elementType, offset, value, sink);
    };

    std::size_t positional = 0;
    for (const auto& element : list->getElements()) {
        if (!element.value) {
            continue;
        }
        std::size_t index = positional;
        if (element.arrayIndex) {
            long idx = *element.arrayIndex;
            if (idx < 0 || idx >= length) {
                continue;
            }
            index = static_cast<std::size_t>(idx);
        }
        if (index >= static_cast<std::size_t>(length)) {
            continue;
        }
        applyElement(index, element.value.get());
        positional = index + 1;
    }
    for (std::size_t i = 0; i < static_cast<std::size_t>(length); ++i) {
        if (!written[i]) {
            sink.onUnwritten(baseOffset + static_cast<int>(i) * stride, elementType);
        }
    }
}

void walkAggregateInitImpl(const type::Type& targetType,
        const ast::InitializerListExpression* list,
        int baseOffset,
        AggregateInitSink& sink) {
    if (targetType.isRecord()) {
        walkRecord(targetType, list, baseOffset, sink);
    } else if (targetType.isArray()) {
        walkArray(targetType, list, baseOffset, sink);
    }
}

} // namespace

void walkAggregateInit(const type::Type& targetType,
        const ast::InitializerListExpression* list,
        int baseOffset,
        AggregateInitSink& sink) {
    walkAggregateInitImpl(targetType, list, baseOffset, sink);
}

} // namespace init_detail
} // namespace semantic_analyzer
