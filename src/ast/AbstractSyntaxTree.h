#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/PendingArrayMemberStore.h"
#include "parser/SyntaxTree.h"
#include "symbols/AnnotationStore.h"
#include "types/IntegerConstant.h"

namespace ast {

class AbstractSyntaxTree: public parser::SyntaxTree {
private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
    PendingArrayMemberStore pendingArrayMembers_;
    symbols::AnnotationStore annotations_;
    // Parse-phase handoff bag for enumerators (not a permanent second authority).
    // Pipeline: LexicalSession.enums (parse) -> this snapshot at build() ->
    // SymbolTable (SA import). Three maps exist for phase boundaries; collapse
    // only when enums gain real scope. Includes enums nested in structs; not
    // nested enum-in-const_exp (unsupported; PE single open-enum counter).
    // SA imports the whole map before the walk (TU-flat; not C declaration-order).
    std::map<std::string, type::IntegerConstant> parseEnumConstants_;

public:
    AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit,
            PendingArrayMemberStore pendingArrayMembers = {});
    virtual ~AbstractSyntaxTree() = default;

    auto begin() const -> decltype(translationUnit.begin());
    auto end() const -> decltype(translationUnit.end());

    PendingArrayMemberStore& pendingArrayMembers() { return pendingArrayMembers_; }
    const PendingArrayMemberStore& pendingArrayMembers() const { return pendingArrayMembers_; }

    symbols::AnnotationStore& annotations() { return annotations_; }

    void setParseEnumConstants(std::map<std::string, type::IntegerConstant> constants) {
        parseEnumConstants_ = std::move(constants);
    }
    const std::map<std::string, type::IntegerConstant>& parseEnumConstants() const {
        return parseEnumConstants_;
    }

    void accept(ast::AbstractSyntaxTreeVisitor& visitor) const;
};

} // namespace ast

#endif // ABSTRACTSYNTAXTREE_H_
