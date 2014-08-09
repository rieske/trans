#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <string>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Pointer: public NonterminalNode {
public:
    Pointer();

    void dereference();
    std::string getExtendedType() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

private:
    std::string extendedType;
};

}

#endif // _PTR_NODE_H_
