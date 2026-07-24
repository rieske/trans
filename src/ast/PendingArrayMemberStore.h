#ifndef AST_PENDINGARRAYMEMBERSTORE_H_
#define AST_PENDINGARRAYMEMBERSTORE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ast/Declarator.h"
#include "types/Type.h"

namespace ast {

// Array member whose bound could not fold at AST-build time (e.g. ARRAY_SIZE of a
// later global). Owned per translation unit (builder context -> AST product).
// Semantic analysis re-folds all groups once after file-scope symbols exist.
struct PendingArrayMember {
    PendingArrayMember(std::string name, type::Type base, std::shared_ptr<Declarator> decl)
            : memberName { std::move(name) },
              baseType { std::move(base) },
              declarator { std::move(decl) } {
    }

    std::string memberName;
    type::Type baseType;
    std::shared_ptr<Declarator> declarator;
};

// One incomplete struct/union type plus the member declarators to re-fold.
struct PendingArrayMemberGroup {
    type::Type structType;
    std::vector<PendingArrayMember> members;
};

// Groups keyed by Type::structureBodyIdentity() (shared StructBody address).
class PendingArrayMemberStore {
public:
    void add(type::Type structType, PendingArrayMember member);

    // All groups still awaiting a successful fold.
    std::vector<PendingArrayMemberGroup>& groups() { return groups_; }
    const std::vector<PendingArrayMemberGroup>& groups() const { return groups_; }

    void clear() { groups_.clear(); }
    bool empty() const { return groups_.empty(); }

private:
    std::vector<PendingArrayMemberGroup> groups_;
};

} // namespace ast

#endif // AST_PENDINGARRAYMEMBERSTORE_H_
