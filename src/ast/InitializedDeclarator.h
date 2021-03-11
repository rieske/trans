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

    bool hasInitializer() const;
    semantic_analyzer::ValueEntry* getInitializerHolder() const;

    translation_unit::Context getContext() const;

    void setHolder(semantic_analyzer::ValueEntry holder);
    semantic_analyzer::ValueEntry* getHolder() const;

private:
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expression> initializer;

    std::unique_ptr<semantic_analyzer::ValueEntry> holder;
};

} // namespace ast

#endif // INITIALIZEDDECLARATOR_H_
