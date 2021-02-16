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

    void accept(SyntaxTreeVisitor& visitor) override;

    void outputXml(std::ostream& stream) const override;
    void outputSource(std::ostream& stream) const override;

private:
    std::unique_ptr<ParseTreeNode> tree;
};

} // namespace parser

#endif // PARSETREE_H_
