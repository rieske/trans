#ifndef STATEMENT_H_
#define STATEMENT_H_

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Block;

class Statement: public AbstractSyntaxTreeNode {
public:
    virtual ~Statement() = default;

    virtual Block* asBlock() { return nullptr; }
};

} // namespace ast

#endif
