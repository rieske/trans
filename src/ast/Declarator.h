#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "DirectDeclarator.h"
#include "Pointer.h"

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {

class Declarator: public AbstractSyntaxTreeNode {
public:
    Declarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Pointer> pointer = nullptr);
    virtual ~Declarator() = default;

    const static std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    translation_unit::Context getContext() const;

private:
    std::unique_ptr<DirectDeclarator> declarator;
    std::unique_ptr<Pointer> pointer;
};

} /* namespace ast */

#endif /* _DECLARATOR_H_ */
