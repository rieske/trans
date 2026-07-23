#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "symbols/ValueEntry.h"
#include "symbols/FunctionEntry.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"

namespace ast {

class FunctionDefinition: public AbstractSyntaxTreeNode {
public:
    FunctionDefinition(DeclarationSpecifiers returnType, std::unique_ptr<Declarator> declarator, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~FunctionDefinition() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitReturnType(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);
    void visitBody(AbstractSyntaxTreeVisitor& visitor);
    void visitBodyChildren(AbstractSyntaxTreeVisitor& visitor);

    void setSymbol(symbols::FunctionEntry symbol);
    void setLocalVariables(std::map<std::string, symbols::ValueEntry> localVariables);
    void setArguments(std::vector<symbols::ValueEntry> arguments);
    void setParameterNames(std::vector<std::string> names);
    const std::vector<std::string>& getParameterNames() const;

    bool hasSymbol() const;
    symbols::FunctionEntry* getSymbol() const;
    std::map<std::string, symbols::ValueEntry> getLocalVariables() const;
    std::vector<symbols::ValueEntry> getArguments() const;

    std::string getName() const;
    const DeclarationSpecifiers& getReturnTypeSpecifiers() const;
    type::Type getDeclaratorType(const type::Type& baseType) const;
    translation_unit::Context getDeclaratorContext() const;

private:
    DeclarationSpecifiers returnType;
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<AbstractSyntaxTreeNode> body;
};

} // namespace ast

#endif // _FUNC_DECL_NODE_H_
