#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "TypeSpecifier.h"

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {
class ParameterDeclaration;
} /* namespace ast */

namespace code_generator {
class FunctionEntry;
} /* namespace code_generator */

class SymbolTable;

namespace ast {

class FunctionDeclaration;
class TerminalSymbol;

class FunctionDefinition: public AbstractSyntaxTreeNode {
public:
    FunctionDefinition(
            TypeSpecifier returnType,
            std::unique_ptr<FunctionDeclaration> declaration,
            std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~FunctionDefinition();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitDeclaration(AbstractSyntaxTreeVisitor& visitor);

    translation_unit::Context getDeclarationContext() const;
    std::string getName() const;
    const std::vector<std::unique_ptr<ParameterDeclaration>>& getDeclaredArguments() const;

    static const std::string ID;

    void setSymbol(code_generator::FunctionEntry symbol);
    code_generator::FunctionEntry* getSymbol() const;

    TypeSpecifier returnType;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

    const std::unique_ptr<FunctionDeclaration> declaration;
private:

    std::unique_ptr<code_generator::FunctionEntry> symbol;
};

}

#endif // _FUNC_DECL_NODE_H_
