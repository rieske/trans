#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "../code_generator/ValueScope.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Block: public AbstractSyntaxTreeNode {
public:
    Block(std::unique_ptr<AbstractSyntaxTreeNode> subblock);
    Block(std::unique_ptr<AbstractSyntaxTreeNode> firstSubblock, std::unique_ptr<AbstractSyntaxTreeNode> secondSubblock);
    virtual ~Block();

    const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& getChildren() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

    void setSymbols(std::map<std::string, code_generator::ValueEntry> symbols);
    std::map<std::string, code_generator::ValueEntry> getSymbols() const;

    void setArguments(std::map<std::string, code_generator::ValueEntry> arguments);
    std::map<std::string, code_generator::ValueEntry> getArguments() const;

private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> children;

    std::map<std::string, code_generator::ValueEntry> symbols;
    std::map<std::string, code_generator::ValueEntry> arguments;
};

}

#endif // _BLOCK_NODE_H_
