#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/AbstractSyntaxTreeNode.h"
#include "parser/SyntaxTree.h"
#include "symbols/AnnotationStore.h"

namespace ast {

class AbstractSyntaxTree: public parser::SyntaxTree {
private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
    symbols::AnnotationStore annotations_;
    // Parse-time enumerators (nested enums included) for SA symbol-table import.
    std::map<std::string, long> parseEnumConstants_;

public:
    AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit);
    virtual ~AbstractSyntaxTree() = default;

    auto begin() const -> decltype(translationUnit.begin());
    auto end() const -> decltype(translationUnit.end());

    symbols::AnnotationStore& annotations() { return annotations_; }
    const symbols::AnnotationStore& annotations() const { return annotations_; }

    void setParseEnumConstants(std::map<std::string, long> constants) {
        parseEnumConstants_ = std::move(constants);
    }
    const std::map<std::string, long>& parseEnumConstants() const {
        return parseEnumConstants_;
    }

    void accept(ast::AbstractSyntaxTreeVisitor& visitor) const;
    void accept(parser::SyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // ABSTRACTSYNTAXTREE_H_
