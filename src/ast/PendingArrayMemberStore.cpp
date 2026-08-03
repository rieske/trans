#include "PendingArrayMemberStore.h"

namespace ast {

void PendingArrayMemberStore::add(type::Type structType, PendingArrayMember member) {
    if (!member.declarator || !structType.isRecord()) {
        return;
    }
    const void* body = structType.structureBodyIdentity();
    if (!body) {
        return;
    }
    for (auto& g : groups_) {
        if (g.structType.structureBodyIdentity() == body) {
            g.members.push_back(std::move(member));
            return;
        }
    }
    groups_.push_back(PendingArrayMemberGroup {
            std::move(structType),
            { std::move(member) } });
}

} // namespace ast
