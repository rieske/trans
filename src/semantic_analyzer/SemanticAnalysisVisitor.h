#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include <memory>
#include <string>
#include <vector>

#include "code_generator/ValueEntry.h"
#include "code_generator/symbol_table.h"
#include "ast/AbstractSyntaxTreeVisitor.h"

namespace translation_unit {
    class Context;
}

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public ast::AbstractSyntaxTreeVisitor {
public:
    SemanticAnalysisVisitor();
    virtual ~SemanticAnalysisVisitor();

    void visit(ast::TypeSpecifier& typeSpecifier) override;

    void visit(ast::ParameterList& parameterList) override;
    void visit(ast::AssignmentExpressionList& expressions) override;
    void visit(ast::DeclarationList& declarations) override;
    void visit(ast::ArrayAccess& arrayAccess) override;
    void visit(ast::FunctionCall& functionCall) override;
    void visit(ast::Term& term) override;
    void visit(ast::PostfixExpression& expression) override;
    void visit(ast::PrefixExpression& expression) override;
    void visit(ast::UnaryExpression& expression) override;
    void visit(ast::TypeCast& expression) override;
    void visit(ast::PointerCast& expression) override;
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
    void visit(ast::IOStatement& statement) override;
    void visit(ast::IfStatement& statement) override;
    void visit(ast::IfElseStatement& statement) override;
    void visit(ast::LoopStatement& statement) override;

    void visit(ast::ForLoopHeader& loopHeader) override;
    void visit(ast::WhileLoopHeader& loopHeader) override;

    void visit(ast::Pointer& pointer) override;

    void visit(ast::Identifier& identifier) override;
    void visit(ast::FunctionDeclaration& declaration) override;
    void visit(ast::ArrayDeclaration& declaration) override;

    void visit(ast::ParameterDeclaration& parameter) override;

    void visit(ast::FunctionDefinition& function) override;

    void visit(ast::VariableDeclaration& declaration) override;
    void visit(ast::VariableDefinition& definition) override;

    void visit(ast::Block& block) override;
    void visit(ast::ListCarrier& listCarrier) override;

    void visit(ast::TranslationUnit& translationUnit) override;

    bool successfulSemanticAnalysis() const;

private:
    void typeCheck(const ast::Type& typeFrom, const ast::Type& typeTo, const translation_unit::Context& context);
    void semanticError(std::string message, const translation_unit::Context& context);

    bool containsSemanticErrors { false };

    code_generator::SymbolTable symbolTable;
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICANALYSISVISITOR_H_ */
