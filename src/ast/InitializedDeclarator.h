#ifndef INITIALIZEDDECLARATOR_H_
#define INITIALIZEDDECLARATOR_H_

#include <memory>
#include <string>

#include "Declarator.h"

namespace ast {

class Declarator;
class Expression;

class InitializedDeclarator: public Declarator {
public:
    InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer = nullptr);
    virtual ~InitializedDeclarator();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

    std::string getName() const;
    int getDereferenceCount() const;

    translation_unit::Context getContext() const;

    void setHolder(code_generator::ValueEntry holder);
    code_generator::ValueEntry* getHolder() const;

private:
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expression> initializer;
};

} /* namespace ast */

#endif /* INITIALIZEDDECLARATOR_H_ */
