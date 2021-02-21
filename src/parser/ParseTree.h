#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <ostream>
#include <memory>

#include "SyntaxTree.h"
#include "SyntaxTreeVisitor.h"
#include "parser/ParseTreeNode.h"

namespace parser {

class ParseTree: public SyntaxTree {
public:
    ParseTree(std::unique_ptr<ParseTreeNode> top);
    virtual ~ParseTree();

    void accept(ParseTreeNodeVisitor& visitor) const;
    void accept(SyntaxTreeVisitor& visitor) override;

private:
    std::unique_ptr<ParseTreeNode> tree;
};

} // namespace parser

#endif // PARSETREE_H_
