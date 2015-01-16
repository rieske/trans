#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"

namespace ast {

class FormalArgument;

typedef std::vector<std::unique_ptr<FormalArgument>> FormalArguments;

class FunctionDeclarator: public DirectDeclarator {
public:
    FunctionDeclarator(std::unique_ptr<Declarator> declarator);
    FunctionDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<FormalArguments> formalArguments);
    virtual ~FunctionDeclarator();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitFormalArguments(AbstractSyntaxTreeVisitor& visitor);

    const FormalArguments& getFormalArguments() const;

private:
    std::unique_ptr<FormalArguments> formalArguments;
};

}
/* namespace ast */

#endif /* FUNCTIONDECLARATION_H_ */
