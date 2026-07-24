#ifndef DEFAULTLABEL_H_
#define DEFAULTLABEL_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"

namespace ast {

class DefaultLabel: public AbstractSyntaxTreeNode {
public:
    DefaultLabel(std::unique_ptr<AbstractSyntaxTreeNode> statement);
    virtual ~DefaultLabel() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLabel(semantic_analyzer::LabelEntry label);
    semantic_analyzer::LabelEntry* getLabel() const;

    const std::unique_ptr<AbstractSyntaxTreeNode> statement;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> label { nullptr };
};

} // namespace ast

#endif // DEFAULTLABEL_H_
