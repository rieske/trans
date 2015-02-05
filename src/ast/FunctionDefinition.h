#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"

namespace code_generator {
class FunctionEntry;
} /* namespace code_generator */

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {

class FunctionDeclarator;

class FunctionDefinition: public AbstractSyntaxTreeNode {
public:
    FunctionDefinition(DeclarationSpecifiers returnType, std::unique_ptr<Declarator> declarator, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~FunctionDefinition() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitReturnType(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);
    void visitBody(AbstractSyntaxTreeVisitor& visitor);

    static const std::string ID;

    void setSymbol(code_generator::FunctionEntry symbol);
    code_generator::FunctionEntry* getSymbol() const;

    std::string getName() const;

private:
    DeclarationSpecifiers returnType;
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<AbstractSyntaxTreeNode> body;

    std::unique_ptr<code_generator::FunctionEntry> symbol;
};

}

#endif // _FUNC_DECL_NODE_H_
