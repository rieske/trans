#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>

#include "DeclarationSpecifiers.h"

namespace ast {
class Declarator;
} /* namespace ast */
namespace semantic_analyzer {
class FunctionEntry;
} /* namespace semantic_analyzer */

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {

class FunctionEntry;

class FunctionDefinition: public AbstractSyntaxTreeNode {
public:
    FunctionDefinition(DeclarationSpecifiers returnType, std::unique_ptr<Declarator> declarator, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~FunctionDefinition() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitReturnType(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);
    void visitBody(AbstractSyntaxTreeVisitor& visitor);

    static const std::string ID;

    void setSymbol(semantic_analyzer::FunctionEntry symbol);
    semantic_analyzer::FunctionEntry* getSymbol() const;

    std::string getName() const;

private:
    DeclarationSpecifiers returnType;
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<AbstractSyntaxTreeNode> body;

    std::unique_ptr<semantic_analyzer::FunctionEntry> symbol;
};

}

#endif // _FUNC_DECL_NODE_H_
