#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <iostream>
#include <memory>
#include <vector>

#include "../parser/SyntaxTree.h"

namespace ast {

class AbstractSyntaxTreeNode;

class AbstractSyntaxTree: public parser::SyntaxTree {
private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit;

public:
    AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit);
    virtual ~AbstractSyntaxTree() = default;

    auto begin() const -> decltype(translationUnit.begin());
    auto end() const -> decltype(translationUnit.end());

    void accept(parser::SyntaxTreeVisitor& visitor) override;

    void outputXml(std::ostream& stream) const override;
    void outputSource(std::ostream& stream) const override;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREE_H_ */
