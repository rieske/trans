#ifndef STATEMENT_H_
#define STATEMENT_H_

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Statement: public AbstractSyntaxTreeNode {
public:
    virtual ~Statement() = default;
};

} // namespace ast

#endif
