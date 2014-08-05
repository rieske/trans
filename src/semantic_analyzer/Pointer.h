#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <string>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Pointer: public NonterminalNode {
public:
    Pointer();

    void dereference();
    int getDereferenceCount() const;

    void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

    static const std::string ID;

private:
    int dereferenceCount { 1 };
};

}

#endif // _PTR_NODE_H_
