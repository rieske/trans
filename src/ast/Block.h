#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "semantic_analyzer/ValueEntry.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {
class Declaration;
} /* namespace ast */

namespace ast {

class Block: public AbstractSyntaxTreeNode {
public:
    Block(std::vector<std::unique_ptr<Declaration>> declarations, std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements);
    virtual ~Block() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    static const std::string ID;

private:
    std::vector<std::unique_ptr<Declaration>> declarations;
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements;
};

}

#endif // _BLOCK_NODE_H_
