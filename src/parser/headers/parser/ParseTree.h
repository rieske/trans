#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <iostream>
#include <memory>

#include "SyntaxTree.h"
#include "SyntaxTreeVisitor.h"

namespace parser {

class ParseTreeNode;

class ParseTree: public SyntaxTree {
public:
    ParseTree(std::unique_ptr<ParseTreeNode> top);
    virtual ~ParseTree() = default;

    void accept(SyntaxTreeVisitor& visitor) override;

    void outputXml(std::ostream& stream) const override;
    void outputSource(std::ostream& stream) const override;

private:
    std::unique_ptr<ParseTreeNode> tree;
};

} /* namespace parser */

#endif /* PARSETREE_H_ */
