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
    // Visit body block contents without Block::accept (no extra scope enter).
    void visitBodyChildren(AbstractSyntaxTreeVisitor& visitor);

    void setSymbol(symbols::FunctionEntry symbol);
    void setLocalVariables(std::map<std::string, symbols::ValueEntry> localVariables);
    void setArguments(std::vector<symbols::ValueEntry> arguments);

    bool hasSymbol() const { return symbol != nullptr; }
    symbols::FunctionEntry* getSymbol() const;
    std::map<std::string, symbols::ValueEntry> getLocalVariables() const;
    std::vector<symbols::ValueEntry> getArguments() const;

    std::string getName() const;

private:
    DeclarationSpecifiers returnType;
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<AbstractSyntaxTreeNode> body;

    // Residual SA→CG frame facts still on the node (FunctionEntry + locals/args).
    // Intentional Phase 0 defer: migrate to store (FunctionFrame / plan) later;
    // do not introduce a second channel alongside these fields.
    std::unique_ptr<symbols::FunctionEntry> symbol;

    std::map<std::string, symbols::ValueEntry> localVariables;
    std::vector<symbols::ValueEntry> arguments;
};

} // namespace ast

#endif // _FUNC_DECL_NODE_H_
