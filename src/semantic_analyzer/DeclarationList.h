#ifndef _DECLS_NODE_H_
#define _DECLS_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"

namespace semantic_analyzer {

class Declaration;

class DeclarationList: public AbstractSyntaxTreeNode {
public:
    DeclarationList(std::unique_ptr<Declaration> declaration);

    void addDeclaration(std::unique_ptr<Declaration> declaration);
    const std::vector<std::unique_ptr<Declaration>>& getDeclarations() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

private:
    std::vector<std::unique_ptr<Declaration>> declarations;
};

}

#endif // _DECLS_NODE_H_
