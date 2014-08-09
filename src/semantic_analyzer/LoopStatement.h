#ifndef LOOPSTATEMENT_H_
#define LOOPSTATEMENT_H_

#include <memory>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class LoopHeader;

class LoopStatement: public NonterminalNode {
public:
    LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *st);
    virtual ~LoopStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLoopEntry(SymbolEntry* loopEntry);
    const SymbolEntry* getLoopEntry() const;
    void setLoopExit(SymbolEntry* loopExit);
    const SymbolEntry* getLoopExit() const;

    const std::unique_ptr<LoopHeader> header;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    SymbolEntry* loopEntry { nullptr };
    SymbolEntry* loopExit { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* LOOPSTATEMENT_H_ */
