#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"

namespace ast {

class FormalArgument;

typedef std::vector<FormalArgument> FormalArguments;

class FunctionDeclarator: public DirectDeclarator {
public:
    FunctionDeclarator(std::unique_ptr<Declarator> declarator);
    FunctionDeclarator(std::unique_ptr<Declarator> declarator, FormalArguments formalArguments);
    virtual ~FunctionDeclarator();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitFormalArguments(AbstractSyntaxTreeVisitor& visitor);

    const FormalArguments& getFormalArguments() const;

private:
    FormalArguments formalArguments;
};

}
/* namespace ast */

#endif /* FUNCTIONDECLARATION_H_ */
