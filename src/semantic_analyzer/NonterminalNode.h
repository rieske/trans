#ifndef _AST_NONTERMINAL_NODE_H_
#define _AST_NONTERMINAL_NODE_H_

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeNode.h"

class SymbolTable;

namespace semantic_analyzer {

class NonterminalNode: public AbstractSyntaxTreeNode {
public:
    virtual ~NonterminalNode() {
    }

    std::vector<Quadruple *> getCode() const override;

protected:
    NonterminalNode() {
    }
    NonterminalNode(SymbolTable *st, unsigned lineNumber);

    void semanticError(std::string description);

    SymbolTable *s_table;
    unsigned sourceLine;

    std::vector<Quadruple *> code;
};

}

#endif // _AST_NONTERMINAL_NODE_H_
