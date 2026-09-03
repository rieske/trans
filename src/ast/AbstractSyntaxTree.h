#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <memory>
#include <vector>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/VlaExpressionTable.h"
#include "symbols/AnnotationStore.h"

namespace ast {

class AbstractSyntaxTree {
private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
    symbols::AnnotationStore annotations_;
    std::shared_ptr<VlaExpressionTable> vlaExpressions_;

public:
    AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit);

    auto begin() const -> decltype(translationUnit.begin());
    auto end() const -> decltype(translationUnit.end());

    symbols::AnnotationStore& annotations() { return annotations_; }

    void setVlaExpressions(std::shared_ptr<VlaExpressionTable> exprs) {
        vlaExpressions_ = std::move(exprs);
    }
    VlaExpressionTable* vlaExpressions() { return vlaExpressions_.get(); }
    const VlaExpressionTable* vlaExpressions() const { return vlaExpressions_.get(); }

    void accept(ast::AbstractSyntaxTreeVisitor& visitor) const;
};

} // namespace ast

#endif // ABSTRACTSYNTAXTREE_H_
