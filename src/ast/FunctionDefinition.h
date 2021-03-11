#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "semantic_analyzer/ValueEntry.h"
#include "semantic_analyzer/FunctionEntry.h"
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

    void setSymbol(semantic_analyzer::FunctionEntry symbol);
    void setLocalVariables(std::map<std::string, semantic_analyzer::ValueEntry> localVariables);
    void setArguments(std::vector<semantic_analyzer::ValueEntry> arguments);

    semantic_analyzer::FunctionEntry* getSymbol() const;
    std::map<std::string, semantic_analyzer::ValueEntry> getLocalVariables() const;
    std::vector<semantic_analyzer::ValueEntry> getArguments() const;

    std::string getName() const;

private:
    DeclarationSpecifiers returnType;
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<AbstractSyntaxTreeNode> body;

    std::unique_ptr<semantic_analyzer::FunctionEntry> symbol;

    std::map<std::string, semantic_analyzer::ValueEntry> localVariables;
    std::vector<semantic_analyzer::ValueEntry> arguments;
};

} // namespace ast

#endif // _FUNC_DECL_NODE_H_
