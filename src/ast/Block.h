#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../semantic_analyzer/ValueEntry.h"
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

    void setSymbols(std::map<std::string, semantic_analyzer::ValueEntry> symbols);
    std::map<std::string, semantic_analyzer::ValueEntry> getSymbols() const;

    void setArguments(std::map<std::string, semantic_analyzer::ValueEntry> arguments);
    std::map<std::string, semantic_analyzer::ValueEntry> getArguments() const;

private:
    std::vector<std::unique_ptr<Declaration>> declarations;
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements;

    std::map<std::string, semantic_analyzer::ValueEntry> symbols;
    std::map<std::string, semantic_analyzer::ValueEntry> arguments;
};

}

#endif // _BLOCK_NODE_H_
