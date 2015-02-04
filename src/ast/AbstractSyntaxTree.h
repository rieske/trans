#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <iostream>
#include <memory>
#include <vector>

#include "../parser/SyntaxTree.h"

namespace ast {

class AbstractSyntaxTreeNode;

class AbstractSyntaxTree: public parser::SyntaxTree {
public:
    AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit);
    virtual ~AbstractSyntaxTree() = default;

    void analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer);

    void outputXml(std::ostream& stream) const override;
    void outputSource(std::ostream& stream) const override;

private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREE_H_ */
