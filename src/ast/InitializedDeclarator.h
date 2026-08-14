#ifndef INITIALIZEDDECLARATOR_H_
#define INITIALIZEDDECLARATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "Declarator.h"
#include "Expression.h"
#include "symbols/AnnotationStore.h"

namespace ast {

class InitializedDeclarator: public AbstractSyntaxTreeNode {
public:
    InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer = nullptr);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);
    // Split so SA can insert the name before analyzing a later initializer in the same declaration.
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);
    void visitInitializer(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    Declarator& getDeclarator() { return *declarator; }
    type::Type getFundamentalType(const type::Type& baseType) const;

    void forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn);

    bool hasInitializer() const;
    Expression* getInitializer() const;
    void setInitializer(std::unique_ptr<Expression> initializer);
    symbols::ValueEntry* getInitializerHolder(symbols::AnnotationStore& store) const;

    translation_unit::Context getContext() const;

    void setHolder(symbols::AnnotationStore& store, symbols::ValueEntry holder);
    symbols::ValueEntry* getHolder(symbols::AnnotationStore& store) const;

private:
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expression> initializer;
};

} // namespace ast

#endif // INITIALIZEDDECLARATOR_H_
