#ifndef _DECLS_NODE_H_
#define _DECLS_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Declarator;

class DeclarationList: public AbstractSyntaxTreeNode {
public:
    DeclarationList(std::unique_ptr<Declarator> declaration);

    void addDeclaration(std::unique_ptr<Declarator> declaration);
    const std::vector<std::unique_ptr<Declarator>>& getDeclarations() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

private:
    std::vector<std::unique_ptr<Declarator>> declarations;
};

}

#endif // _DECLS_NODE_H_
