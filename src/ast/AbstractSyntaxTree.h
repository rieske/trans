#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <iostream>
#include <memory>

#include "../parser/SyntaxTree.h"

namespace ast {

class AbstractSyntaxTreeNode;

class AbstractSyntaxTree: public parser::SyntaxTree {
public:
    AbstractSyntaxTree(std::unique_ptr<AbstractSyntaxTreeNode> top);
    virtual ~AbstractSyntaxTree();

    void analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer);

    void outputXml(std::ostream& stream) const override;
    void outputSource(std::ostream& stream) const override;

private:
    std::unique_ptr<AbstractSyntaxTreeNode> tree;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREE_H_ */
