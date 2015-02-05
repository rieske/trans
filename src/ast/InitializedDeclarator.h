#ifndef INITIALIZEDDECLARATOR_H_
#define INITIALIZEDDECLARATOR_H_

#include <memory>
#include <string>

#include "Declarator.h"
#include "Expression.h"

namespace ast {

class InitializedDeclarator: public AbstractSyntaxTreeNode {
public:
    InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer = nullptr);
    virtual ~InitializedDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    static const std::string ID;

    std::string getName() const;
    int getDereferenceCount() const;

    bool hasInitializer() const;
    code_generator::ValueEntry* getInitializerHolder() const;

    translation_unit::Context getContext() const;

    void setHolder(code_generator::ValueEntry holder);
    code_generator::ValueEntry* getHolder() const;

private:
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expression> initializer;

    std::unique_ptr<code_generator::ValueEntry> holder;
};

} /* namespace ast */

#endif /* INITIALIZEDDECLARATOR_H_ */
