#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>

#include "TypeSpecifier.h"

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

    static const std::string ID;

    TypeSpecifier returnType;
    const std::unique_ptr<FunctionDeclaration> declaration;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

    void setSymbol(code_generator::FunctionEntry symbol);
    code_generator::FunctionEntry* getSymbol() const;

private:
    std::unique_ptr<code_generator::FunctionEntry> symbol;
};

}

#endif // _FUNC_DECL_NODE_H_
