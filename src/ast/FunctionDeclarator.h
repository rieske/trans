#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"

namespace ast {
class FormalArgument;
} /* namespace ast */

namespace ast {

typedef std::vector<std::unique_ptr<FormalArgument>> FormalArguments;

class ParameterList;

class FunctionDeclarator: public DirectDeclarator {
public:
    FunctionDeclarator(std::unique_ptr<Declarator> declarator);
    FunctionDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<FormalArguments> formalArguments);
    virtual ~FunctionDeclarator();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::unique_ptr<FormalArguments> formalArguments;
};

}
/* namespace ast */

#endif /* FUNCTIONDECLARATION_H_ */
