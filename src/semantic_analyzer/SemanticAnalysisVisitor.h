#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include <string>

#include "SymbolTable.h"
#include "ast/AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    SemanticAnalysisVisitor(std::ostream* errorStream);
    virtual ~SemanticAnalysisVisitor();

    void visit(ast::DeclarationSpecifiers& declarationSpecifiers) override;
    void visit(ast::Declaration& declaration) override;

    void visit(ast::Declarator& declarator) override;
    void visit(ast::InitializedDeclarator& declarator) override;

    void visit(ast::ArrayAccess& arrayAccess) override;
    void visit(ast::FunctionCall& functionCall) override;
    void visit(ast::IdentifierExpression& identifier) override;
    void visit(ast::ConstantExpression& constant) override;
    void visit(ast::StringLiteralExpression& stringLiteral) override;
    void visit(ast::PostfixExpression& expression) override;
    void visit(ast::PrefixExpression& expression) override;
    void visit(ast::UnaryExpression& expression) override;
    void visit(ast::TypeCast& expression) override;
    void visit(ast::ArithmeticExpression& expression) override;
    void visit(ast::ShiftExpression& expression) override;
    void visit(ast::ComparisonExpression& expression) override;
    void visit(ast::BitwiseExpression& expression) override;
    void visit(ast::LogicalAndExpression& expression) override;
    void visit(ast::LogicalOrExpression& expression) override;
    void visit(ast::AssignmentExpression& expression) override;
    void visit(ast::ExpressionList& expression) override;

    void visit(ast::Operator& op) override;

    void visit(ast::JumpStatement& statement) override;
    void visit(ast::ReturnStatement& statement) override;
    void visit(ast::VoidReturnStatement& statement) override;
    void visit(ast::IOStatement& statement) override;
    void visit(ast::IfStatement& statement) override;
    void visit(ast::IfElseStatement& statement) override;
    void visit(ast::LoopStatement& statement) override;

    void visit(ast::ForLoopHeader& loopHeader) override;
    void visit(ast::WhileLoopHeader& loopHeader) override;

    void visit(ast::Pointer& pointer) override;

    void visit(ast::Identifier& identifier) override;
    void visit(ast::FunctionDeclarator& declarator) override;
    void visit(ast::ArrayDeclarator& declaration) override;

    void visit(ast::FormalArgument& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::Block& block) override;

    bool successfulSemanticAnalysis() const;
    std::map<std::string, std::string> getConstants() const;

    void printSymbolTable() const;

private:
    void typeCheck(const ast::FundamentalType& typeFrom, const ast::FundamentalType& typeTo, const translation_unit::Context& context);
    void semanticError(std::string message, const translation_unit::Context& context);

    std::vector<std::string> argumentNames;

    bool containsSemanticErrors { false };
    std::ostream* errorStream;

    SymbolTable symbolTable;
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_H_
