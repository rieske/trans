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

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    type::Type getFundamentalType(const type::Type& baseType);

    Declarator* getDeclarator() const;
    bool hasInitializer() const;
    Expression* getInitializer() const;

    translation_unit::Context getContext() const;

private:
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expression> initializer;
};

} // namespace ast

#endif // INITIALIZEDDECLARATOR_H_
