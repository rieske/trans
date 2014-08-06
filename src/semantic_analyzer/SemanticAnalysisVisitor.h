#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include <memory>

#include "AbstractSyntaxTreeVisitor.h"
#include "AssignmentExpressionList.h"
#include "Block.h"
#include "DeclarationList.h"
#include "Identifier.h"
#include "JumpStatement.h"
#include "ListCarrier.h"
#include "ParameterList.h"
#include "ReturnStatement.h"
#include "TranslationUnit.h"

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public AbstractSyntaxTreeVisitor {
public:
    SemanticAnalysisVisitor();
    virtual ~SemanticAnalysisVisitor();

    void visit(const TypeSpecifier& typeSpecifier) override;

    void visit(const ParameterList& parameterList) override;
    void visit(const AssignmentExpressionList& expressions) override;
    void visit(const DeclarationList& declarations) override;
    void visit(const ArrayAccess& arrayAccess) override;
    void visit(const FunctionCall& functionCall) override;
    void visit(const NoArgFunctionCall& functionCall) override;
    void visit(const Term& term) override;
    void visit(const PostfixExpression& expression) override;
    void visit(const PrefixExpression& expression) override;
    void visit(const UnaryExpression& expression) override;
    void visit(const TypeCast& expression) override;
    void visit(const PointerCast& expression) override;
    void visit(const ArithmeticExpression& expression) override;
    void visit(const ShiftExpression& expression) override;
    void visit(const ComparisonExpression& expression) override;
    void visit(const BitwiseExpression& expression) override;
    void visit(const LogicalAndExpression& expression) override;
    void visit(const LogicalOrExpression& expression) override;
    void visit(const AssignmentExpression& expression) override;
    void visit(const ExpressionList& expression) override;

    void visit(const JumpStatement& statement) override;
    void visit(const ReturnStatement& statement) override;
    void visit(const IOStatement& statement) override;
    void visit(const IfStatement& statement) override;
    void visit(const IfElseStatement& statement) override;
    void visit(const LoopStatement& statement) override;

    void visit(const ForLoopHeader& loopHeader) override;
    void visit(const WhileLoopHeader& loopHeader) override;

    void visit(const Pointer& pointer) override;

    void visit(const Identifier& identifier) override;
    void visit(const FunctionDeclaration& declaration) override;
    void visit(const ArrayDeclaration& declaration) override;

    void visit(const ParameterDeclaration& parameter) override;

    void visit(const FunctionDefinition& function) override;

    void visit(const VariableDeclaration& declaration) override;
    void visit(const VariableDefinition& definition) override;

    void visit(const Block& block) override;
    void visit(const ListCarrier& listCarrier) override;

    void visit(const TranslationUnit& translationUnit) override;

private:
    void error(std::string message, size_t lineNumber);

    bool containsSemanticErrors { false };

    std::unique_ptr<SymbolTable> symbolTable;

    SymbolTable* currentScope;
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICANALYSISVISITOR_H_ */
