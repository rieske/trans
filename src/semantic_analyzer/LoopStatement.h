#ifndef LOOPSTATEMENT_H_
#define LOOPSTATEMENT_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"

namespace semantic_analyzer {

class LoopHeader;

class LoopStatement: public AbstractSyntaxTreeNode {
public:
    LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~LoopStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    const std::unique_ptr<LoopHeader> header;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;
};

} /* namespace semantic_analyzer */

#endif /* LOOPSTATEMENT_H_ */
