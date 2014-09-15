#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <string>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Pointer: public AbstractSyntaxTreeNode {
public:
    Pointer();

    void dereference();
    int getDereferenceCount() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

private:
    int dereferenceCount { 1 };
};

}

#endif // _PTR_NODE_H_
