#include "ContextualSyntaxNodeBuilder.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclaration.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "BasicType.h"
#include "BitwiseExpression.h"
#include "Block.h"
#include "ComparisonExpression.h"
#include "DeclarationList.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FunctionCall.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "Identifier.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "IOStatement.h"
#include "JumpStatement.h"
#include "ListCarrier.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "ParameterDeclaration.h"
#include "ParameterList.h"
#include "Pointer.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ReturnStatement.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TranslationUnit.h"
#include "TypeCast.h"
#include "TypeSpecifier.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"
#include "WhileLoopHeader.h"

using std::unique_ptr;

using sequence = std::vector<std::string>;

static const std::string UNMATCHED { "<unmatched>" };
static const std::string MATCHED { "<matched>" };
static const std::string DIRECT_DECLARATION { "<dir_decl>" };
static const std::string STATEMENT { "<stmt>" };
static const std::string STATEMENTS { "<statements>" };
static const std::string VAR_DECLARATION { "<var_decl>" };
static const std::string VAR_DECLARATIONS { "<var_decls>" };
static const std::string FUNCTION_DECLARATION { "<func_decl>" };
static const std::string FUNCTION_DECLARATIONS { "<func_decls>" };
static const std::string TRANSLATION_UNIT { "<program>" };

static const std::string TYPE_SPECIFIER { "<type_spec>" };
static const std::string UNARY_OPERATOR { "<u_op>" };
static const std::string MULTIPLICATION_OPERATOR { "<m_op>" };
static const std::string ADDITION_OPERATOR { "<add_op>" };
static const std::string SHIFT_OPERATOR { "<s_op>" };
static const std::string COMPARISON_OPERATOR { "<ml_op>" };
static const std::string EQUALITY_OPERATOR { "<eq_op>" };
static const std::string ASSIGNMENT_OPERATOR { "<a_op>" };

namespace semantic_analyzer {

ContextualSyntaxNodeBuilder::ContextualSyntaxNodeBuilder() {
    nodeCreatorRegistry[Term::ID][sequence { "(", Expression::ID, ")" }] =
            ContextualSyntaxNodeBuilder::parenthesizedExpression;
    nodeCreatorRegistry[Term::ID][sequence { "id" }] = ContextualSyntaxNodeBuilder::term;
    nodeCreatorRegistry[Term::ID][sequence { "int_const" }] = ContextualSyntaxNodeBuilder::term;
    nodeCreatorRegistry[Term::ID][sequence { "float_const" }] = ContextualSyntaxNodeBuilder::term;
    nodeCreatorRegistry[Term::ID][sequence { "literal" }] = ContextualSyntaxNodeBuilder::term;
    nodeCreatorRegistry[Term::ID][sequence { "string" }] = ContextualSyntaxNodeBuilder::term;

    nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "[", Expression::ID, "]" }] =
            ContextualSyntaxNodeBuilder::arrayAccess;
    nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "(", AssignmentExpressionList::ID, ")" }] =
            ContextualSyntaxNodeBuilder::functionCall;
    nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "(", ")" }] =
            ContextualSyntaxNodeBuilder::noargFunctionCall;
    nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "++" }] =
            ContextualSyntaxNodeBuilder::postfixIncrementDecrement;
    nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "--" }] =
            ContextualSyntaxNodeBuilder::postfixIncrementDecrement;
    nodeCreatorRegistry[PostfixExpression::ID][sequence { Term::ID }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[UnaryExpression::ID][sequence { "++", UnaryExpression::ID }] =
            ContextualSyntaxNodeBuilder::prefixIncrementDecrement;
    nodeCreatorRegistry[UnaryExpression::ID][sequence { "--", UnaryExpression::ID }] =
            ContextualSyntaxNodeBuilder::prefixIncrementDecrement;
    nodeCreatorRegistry[UnaryExpression::ID][sequence { "<u_op>", TypeCast::ID }] =
            ContextualSyntaxNodeBuilder::unaryExpression;
    nodeCreatorRegistry[UnaryExpression::ID][sequence { PostfixExpression::ID }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[TypeCast::ID][sequence { "(", "<type_spec>", ")", TypeCast::ID }] =
            ContextualSyntaxNodeBuilder::typeCast;
    nodeCreatorRegistry[TypeCast::ID][sequence { "(", "<type_spec>", Pointer::ID, ")", TypeCast::ID }] =
            ContextualSyntaxNodeBuilder::pointerCast;
    nodeCreatorRegistry[TypeCast::ID][sequence { UnaryExpression::ID }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][sequence { ArithmeticExpression::MULTIPLICATION, "<m_op>",
            TypeCast::ID }] = ContextualSyntaxNodeBuilder::arithmeticExpression;
    nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][sequence { TypeCast::ID }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ArithmeticExpression::ADDITION][sequence { ArithmeticExpression::ADDITION, "<add_op>",
            ArithmeticExpression::MULTIPLICATION }] = ContextualSyntaxNodeBuilder::arithmeticExpression;
    nodeCreatorRegistry[ArithmeticExpression::ADDITION][sequence { ArithmeticExpression::MULTIPLICATION }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ShiftExpression::ID][sequence { ShiftExpression::ID, "<s_op>", ArithmeticExpression::ADDITION }] =
            ContextualSyntaxNodeBuilder::shiftExpression;
    nodeCreatorRegistry[ShiftExpression::ID][sequence { ArithmeticExpression::ADDITION }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ComparisonExpression::DIFFERENCE][sequence { ComparisonExpression::DIFFERENCE, "<ml_op>",
            ShiftExpression::ID }] = ContextualSyntaxNodeBuilder::comparisonExpression;
    nodeCreatorRegistry[ComparisonExpression::DIFFERENCE][sequence { ShiftExpression::ID }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ComparisonExpression::EQUALITY][sequence { ComparisonExpression::EQUALITY, "<eq_op>",
            ComparisonExpression::DIFFERENCE }] = ContextualSyntaxNodeBuilder::comparisonExpression;
    nodeCreatorRegistry[ComparisonExpression::EQUALITY][sequence { ComparisonExpression::DIFFERENCE }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[BitwiseExpression::AND][sequence { BitwiseExpression::ID, "&", ComparisonExpression::EQUALITY }] =
            ContextualSyntaxNodeBuilder::bitwiseExpression;
    nodeCreatorRegistry[BitwiseExpression::AND][sequence { ComparisonExpression::EQUALITY }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[BitwiseExpression::XOR][sequence { BitwiseExpression::XOR, "^", BitwiseExpression::AND }] =
            ContextualSyntaxNodeBuilder::bitwiseExpression;
    nodeCreatorRegistry[BitwiseExpression::XOR][sequence { BitwiseExpression::AND }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[BitwiseExpression::OR][sequence { BitwiseExpression::OR, "|", BitwiseExpression::XOR }] =
            ContextualSyntaxNodeBuilder::bitwiseExpression;
    nodeCreatorRegistry[BitwiseExpression::OR][sequence { BitwiseExpression::XOR }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[LogicalAndExpression::ID][sequence { LogicalAndExpression::ID, "&&", BitwiseExpression::OR }] =
            ContextualSyntaxNodeBuilder::logicalAndExpression;
    nodeCreatorRegistry[LogicalAndExpression::ID][sequence { BitwiseExpression::OR }] =
            ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[LogicalOrExpression::ID][sequence { LogicalOrExpression::ID, "||", Expression::ID }] =
            ContextualSyntaxNodeBuilder::logicalOrExpression;
    nodeCreatorRegistry[LogicalOrExpression::ID][sequence { LogicalAndExpression::ID }] =
            ContextualSyntaxNodeBuilder::backpatchExpression;

    nodeCreatorRegistry[AssignmentExpression::ID][sequence { UnaryExpression::ID, "<a_op>", AssignmentExpression::ID }] =
            ContextualSyntaxNodeBuilder::assignmentExpression;
    nodeCreatorRegistry[AssignmentExpression::ID][sequence { LogicalOrExpression::ID }] =
            ContextualSyntaxNodeBuilder::backpatchExpression;

    nodeCreatorRegistry[AssignmentExpressionList::ID][sequence { AssignmentExpression::ID }] =
            ContextualSyntaxNodeBuilder::createAssignmentExpressionList;
    nodeCreatorRegistry[AssignmentExpressionList::ID][sequence { AssignmentExpressionList::ID, ",",
            AssignmentExpression::ID }] = ContextualSyntaxNodeBuilder::addAssignmentExpressionToList;

    nodeCreatorRegistry[Expression::ID][sequence { Expression::ID, ",", AssignmentExpression::ID }] =
            ContextualSyntaxNodeBuilder::expressionList;
    nodeCreatorRegistry[Expression::ID][sequence { AssignmentExpression::ID }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[JumpStatement::ID][sequence { "continue", ";" }] =
            ContextualSyntaxNodeBuilder::loopJumpStatement;
    nodeCreatorRegistry[JumpStatement::ID][sequence { "break", ";" }] = ContextualSyntaxNodeBuilder::loopJumpStatement;
    nodeCreatorRegistry[JumpStatement::ID][sequence { "return", Expression::ID, ";" }] =
            ContextualSyntaxNodeBuilder::returnStatement;

    nodeCreatorRegistry[IOStatement::ID][sequence { "output", Expression::ID, ";" }] =
            ContextualSyntaxNodeBuilder::inputOutputStatement;
    nodeCreatorRegistry[IOStatement::ID][sequence { "input", Expression::ID, ";" }] =
            ContextualSyntaxNodeBuilder::inputOutputStatement;

    nodeCreatorRegistry[LoopHeader::ID][sequence { "while", "(", Expression::ID, ")" }] =
            ContextualSyntaxNodeBuilder::whileLoopHeader;
    nodeCreatorRegistry[LoopHeader::ID][sequence { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID,
            ")" }] = ContextualSyntaxNodeBuilder::forLoopHeader;

    nodeCreatorRegistry[UNMATCHED][sequence { "if", "(", Expression::ID, ")", STATEMENT }] =
            ContextualSyntaxNodeBuilder::ifStatement;
    nodeCreatorRegistry[UNMATCHED][sequence { "if", "(", Expression::ID, ")", MATCHED, "else", UNMATCHED }] =
            ContextualSyntaxNodeBuilder::ifElseStatement;
    nodeCreatorRegistry[UNMATCHED][sequence { LoopHeader::ID, UNMATCHED }] = ContextualSyntaxNodeBuilder::loopStatement;

    nodeCreatorRegistry[MATCHED][sequence { Expression::ID, ";" }] = ContextualSyntaxNodeBuilder::expressionStatement;
    nodeCreatorRegistry[MATCHED][sequence { ";" }] = ContextualSyntaxNodeBuilder::emptyStatement;
    nodeCreatorRegistry[MATCHED][sequence { IOStatement::ID }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[MATCHED][sequence { Block::ID }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[MATCHED][sequence { JumpStatement::ID }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[MATCHED][sequence { "if", "(", Expression::ID, ")", MATCHED, "else", MATCHED }] =
            ContextualSyntaxNodeBuilder::ifElseStatement;
    nodeCreatorRegistry[MATCHED][sequence { LoopHeader::ID, MATCHED }] = ContextualSyntaxNodeBuilder::loopStatement;

    nodeCreatorRegistry[ParameterDeclaration::ID][sequence { "<type_spec>", Declaration::ID }] =
            ContextualSyntaxNodeBuilder::parameterDeclaration;

    nodeCreatorRegistry[ParameterList::ID][sequence { ParameterDeclaration::ID }] =
            ContextualSyntaxNodeBuilder::parameterList;
    nodeCreatorRegistry[ParameterList::ID][sequence { ParameterList::ID, ",", ParameterDeclaration::ID }] =
            ContextualSyntaxNodeBuilder::addParameterToList;

    nodeCreatorRegistry[DIRECT_DECLARATION][sequence { "(", Declaration::ID, ")" }] =
            ContextualSyntaxNodeBuilder::parenthesizedExpression;
    nodeCreatorRegistry[DIRECT_DECLARATION][sequence { "id" }] = ContextualSyntaxNodeBuilder::identifierDeclaration;
    nodeCreatorRegistry[DIRECT_DECLARATION][sequence { DIRECT_DECLARATION, "(", ParameterList::ID, ")" }] =
            ContextualSyntaxNodeBuilder::functionDeclaration;
    nodeCreatorRegistry[DIRECT_DECLARATION][sequence { DIRECT_DECLARATION, "[", LogicalOrExpression::ID, "]" }] =
            ContextualSyntaxNodeBuilder::arrayDeclaration;
    nodeCreatorRegistry[DIRECT_DECLARATION][sequence { DIRECT_DECLARATION, "(", ")" }] =
            ContextualSyntaxNodeBuilder::noargFunctionDeclaration;

    nodeCreatorRegistry[Pointer::ID][sequence { "*" }] = ContextualSyntaxNodeBuilder::pointer;
    nodeCreatorRegistry[Pointer::ID][sequence { Pointer::ID, "*" }] = ContextualSyntaxNodeBuilder::pointerToPointer;

    nodeCreatorRegistry[Block::ID][sequence { "{", VAR_DECLARATIONS, STATEMENTS, "}" }] =
            ContextualSyntaxNodeBuilder::doubleBlock;
    nodeCreatorRegistry[Block::ID][sequence { "{", VAR_DECLARATIONS, "}" }] = ContextualSyntaxNodeBuilder::singleBlock;
    nodeCreatorRegistry[Block::ID][sequence { "{", STATEMENTS, "}" }] = ContextualSyntaxNodeBuilder::singleBlock;
    nodeCreatorRegistry[Block::ID][sequence { "{", "}" }] = ContextualSyntaxNodeBuilder::parenthesizedExpression;

    nodeCreatorRegistry[Declaration::ID][sequence { Pointer::ID, DIRECT_DECLARATION }] =
            ContextualSyntaxNodeBuilder::pointerToDeclaration;
    nodeCreatorRegistry[Declaration::ID][sequence { DIRECT_DECLARATION }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[DeclarationList::ID][sequence { Declaration::ID }] =
            ContextualSyntaxNodeBuilder::declarationList;
    nodeCreatorRegistry[DeclarationList::ID][sequence { DeclarationList::ID, ",", Declaration::ID }] =
            ContextualSyntaxNodeBuilder::addDeclarationToList;

    nodeCreatorRegistry[VariableDeclaration::ID][sequence { "<type_spec>", DeclarationList::ID, ";" }] =
            ContextualSyntaxNodeBuilder::variableDeclaration;
    nodeCreatorRegistry[VariableDeclaration::ID][sequence { "<type_spec>", DeclarationList::ID, "=",
            AssignmentExpression::ID, ";" }] = ContextualSyntaxNodeBuilder::variableDefinition;

    nodeCreatorRegistry[FunctionDefinition::ID][sequence { "<type_spec>", Declaration::ID, Block::ID }] =
            ContextualSyntaxNodeBuilder::functionDefinition;

    nodeCreatorRegistry[STATEMENTS][sequence { STATEMENT }] = ContextualSyntaxNodeBuilder::newListCarrier;
    nodeCreatorRegistry[STATEMENTS][sequence { STATEMENTS, STATEMENT }] = ContextualSyntaxNodeBuilder::addToListCarrier;

    nodeCreatorRegistry[VAR_DECLARATIONS][sequence { VAR_DECLARATION }] = ContextualSyntaxNodeBuilder::newListCarrier;
    nodeCreatorRegistry[VAR_DECLARATIONS][sequence { VAR_DECLARATIONS, VAR_DECLARATION }] =
            ContextualSyntaxNodeBuilder::addToListCarrier;

    nodeCreatorRegistry[FUNCTION_DECLARATIONS][sequence { FUNCTION_DECLARATION }] =
            ContextualSyntaxNodeBuilder::newListCarrier;
    nodeCreatorRegistry[FUNCTION_DECLARATIONS][sequence { FUNCTION_DECLARATIONS, FUNCTION_DECLARATION }] =
            ContextualSyntaxNodeBuilder::addToListCarrier;

    nodeCreatorRegistry[TRANSLATION_UNIT][sequence { FUNCTION_DECLARATIONS }] =
            ContextualSyntaxNodeBuilder::functionsTranslationUnit;
    nodeCreatorRegistry[TRANSLATION_UNIT][sequence { VAR_DECLARATIONS, FUNCTION_DECLARATIONS }] =
            ContextualSyntaxNodeBuilder::variablesFunctionsTranslationUnit;

    nodeCreatorRegistry[STATEMENT][sequence { MATCHED }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[STATEMENT][sequence { UNMATCHED }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[TYPE_SPECIFIER][sequence { "int" }] = ContextualSyntaxNodeBuilder::newIntegerType;
    nodeCreatorRegistry[TYPE_SPECIFIER][sequence { "char" }] = ContextualSyntaxNodeBuilder::newCharacterType;
    nodeCreatorRegistry[TYPE_SPECIFIER][sequence { "void" }] = ContextualSyntaxNodeBuilder::newVoidType;
    nodeCreatorRegistry[TYPE_SPECIFIER][sequence { "float" }] = ContextualSyntaxNodeBuilder::newFloatType;

    nodeCreatorRegistry[UNARY_OPERATOR][sequence { "&" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[UNARY_OPERATOR][sequence { "*" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[UNARY_OPERATOR][sequence { "+" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[UNARY_OPERATOR][sequence { "-" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[UNARY_OPERATOR][sequence { "!" }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[MULTIPLICATION_OPERATOR][sequence { "*" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[MULTIPLICATION_OPERATOR][sequence { "/" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[MULTIPLICATION_OPERATOR][sequence { "%" }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ADDITION_OPERATOR][sequence { "+" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ADDITION_OPERATOR][sequence { "-" }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[SHIFT_OPERATOR][sequence { ">>" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[SHIFT_OPERATOR][sequence { "<<" }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[COMPARISON_OPERATOR][sequence { "<" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[COMPARISON_OPERATOR][sequence { ">" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[COMPARISON_OPERATOR][sequence { "<=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[COMPARISON_OPERATOR][sequence { ">=" }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[EQUALITY_OPERATOR][sequence { "==" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[EQUALITY_OPERATOR][sequence { "!=" }] = ContextualSyntaxNodeBuilder::doNothing;

    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "+=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "-=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "*=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "/=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "%=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "&=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "^=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "|=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { "<<=" }] = ContextualSyntaxNodeBuilder::doNothing;
    nodeCreatorRegistry[ASSIGNMENT_OPERATOR][sequence { ">>=" }] = ContextualSyntaxNodeBuilder::doNothing;

}

ContextualSyntaxNodeBuilder::~ContextualSyntaxNodeBuilder() {
}

void ContextualSyntaxNodeBuilder::updateContext(std::string definingSymbol, const std::vector<std::string>& production,
        AbstractSyntaxTreeBuilderContext& context) const {
    try {
        nodeCreatorRegistry.at(definingSymbol).at(production)(context);
    } catch (std::out_of_range& exception) {
        noCreatorDefined(definingSymbol, production);
    }
}

void ContextualSyntaxNodeBuilder::noCreatorDefined(std::string definingSymbol,
        const std::vector<std::string>& production) {
    std::ostringstream productionString;
    for (auto& symbol : production) {
        productionString << symbol << " ";
    }
    throw std::runtime_error { "no AST creator defined for production `" + definingSymbol + " ::= "
            + productionString.str() + "`" };
}

void ContextualSyntaxNodeBuilder::doNothing(AbstractSyntaxTreeBuilderContext&) {
}

void ContextualSyntaxNodeBuilder::parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
}

void ContextualSyntaxNodeBuilder::term(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(
            std::unique_ptr<Expression> { new Term(context.popTerminal(), context.scope(), context.line()) });
}

void ContextualSyntaxNodeBuilder::arrayAccess(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ]
    context.popTerminal(); // [
    auto subscriptExpression = context.popExpression();
    auto postfixExpression = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new ArrayAccess(std::move(postfixExpression), std::move(subscriptExpression),
                    context.scope(), context.line()) });
}

void ContextualSyntaxNodeBuilder::functionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(
            std::unique_ptr<Expression> { new FunctionCall(context.popExpression(),
                    context.popAssignmentExpressionList(), context.scope(), context.line()) });
}

void ContextualSyntaxNodeBuilder::noargFunctionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(
            std::unique_ptr<Expression> { new FunctionCall(context.popExpression(),
                    std::unique_ptr<AssignmentExpressionList> { new AssignmentExpressionList { } }, context.scope(),
                    context.line()) });
}

void ContextualSyntaxNodeBuilder::postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(
            std::unique_ptr<Expression> { new PostfixExpression(context.popExpression(), context.popTerminal(),
                    context.scope()) });
}

void ContextualSyntaxNodeBuilder::prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(
            std::unique_ptr<Expression> { new PrefixExpression(context.popTerminal(), context.popExpression(),
                    context.scope()) });
}

void ContextualSyntaxNodeBuilder::unaryExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(
            std::unique_ptr<Expression> { new UnaryExpression(context.popTerminal(), context.popExpression(),
                    context.scope()) });
}

void ContextualSyntaxNodeBuilder::typeCast(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.pushExpression(
            std::unique_ptr<Expression> { new TypeCast(context.popTypeSpecifier(), context.popExpression(),
                    context.scope()) });
    context.popTerminal(); // (
}

void ContextualSyntaxNodeBuilder::pointerCast(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.pushExpression(
            std::unique_ptr<Expression> { new PointerCast(context.popTypeSpecifier(), context.popPointer(),
                    context.popExpression(), context.scope()) });
    context.popTerminal(); // (
}

void ContextualSyntaxNodeBuilder::arithmeticExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new ArithmeticExpression(std::move(leftHandSide), context.popTerminal(),
                    std::move(rightHandSide), context.scope()) });
}

void ContextualSyntaxNodeBuilder::shiftExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto additionExpression = context.popExpression();
    auto shiftExpression = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new ShiftExpression(std::move(shiftExpression), context.popTerminal(),
                    std::move(additionExpression), context.scope()) });
}

void ContextualSyntaxNodeBuilder::comparisonExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new ComparisonExpression(std::move(leftHandSide), context.popTerminal(),
                    std::move(rightHandSide), context.scope()) });
}

void ContextualSyntaxNodeBuilder::bitwiseExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new BitwiseExpression(std::move(leftHandSide), context.popTerminal(),
                    std::move(rightHandSide), context.scope()) });
}

void ContextualSyntaxNodeBuilder::logicalAndExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new LogicalAndExpression(std::move(leftHandSide), std::move(rightHandSide),
                    context.scope(), context.line()) });
}

void ContextualSyntaxNodeBuilder::logicalOrExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new LogicalOrExpression(std::move(leftHandSide), std::move(rightHandSide),
                    context.scope(), context.line()) });
}

void ContextualSyntaxNodeBuilder::assignmentExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new AssignmentExpression(std::move(leftHandSide), context.popTerminal(),
                    std::move(rightHandSide), context.scope()) });
}

void ContextualSyntaxNodeBuilder::backpatchExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto backpatchExpression = context.popExpression();
    backpatchExpression->backpatch();
    context.pushExpression(std::move(backpatchExpression));
}

void ContextualSyntaxNodeBuilder::createAssignmentExpressionList(AbstractSyntaxTreeBuilderContext& context) {
    context.pushAssignmentExpressionList(
            std::unique_ptr<AssignmentExpressionList> { new AssignmentExpressionList(context.popExpression()) });
}

void ContextualSyntaxNodeBuilder::addAssignmentExpressionToList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto assignmentExpressions = context.popAssignmentExpressionList();
    assignmentExpressions->addExpression(context.popExpression());
    context.pushAssignmentExpressionList(std::move(assignmentExpressions));
}

void ContextualSyntaxNodeBuilder::expressionList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(
            std::unique_ptr<Expression> { new ExpressionList(std::move(leftHandSide), std::move(rightHandSide),
                    context.scope(), context.line()) });
}

void ContextualSyntaxNodeBuilder::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new JumpStatement(context.popTerminal()) });
    context.popTerminal();
}

void ContextualSyntaxNodeBuilder::returnStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new ReturnStatement(context.popExpression()) });
}

void ContextualSyntaxNodeBuilder::inputOutputStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new IOStatement(context.popTerminal(), context.popExpression()) });
}

void ContextualSyntaxNodeBuilder::whileLoopHeader(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushLoopHeader(
            std::unique_ptr<LoopHeader> { new WhileLoopHeader(context.popExpression(), context.scope()) });
}

void ContextualSyntaxNodeBuilder::forLoopHeader(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto increment = context.popExpression();
    auto clause = context.popExpression();
    auto initialization = context.popExpression();
    context.pushLoopHeader(
            std::unique_ptr<LoopHeader> { new ForLoopHeader(std::move(initialization), std::move(clause),
                    std::move(increment), context.scope()) });
}

void ContextualSyntaxNodeBuilder::ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new IfStatement(context.popExpression(), context.popStatement(),
                    context.scope()) });
}

void ContextualSyntaxNodeBuilder::ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto falsyStatement = context.popStatement();
    auto truthyStatement = context.popStatement();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new IfElseStatement(context.popExpression(),
                    std::move(truthyStatement), std::move(falsyStatement), context.scope()) });
}

void ContextualSyntaxNodeBuilder::loopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new LoopStatement(context.popLoopHeader(), context.popStatement(),
                    context.scope()) });
}

void ContextualSyntaxNodeBuilder::expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(context.popExpression());
}

void ContextualSyntaxNodeBuilder::emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
}

void ContextualSyntaxNodeBuilder::parameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushParameter(
            std::unique_ptr<ParameterDeclaration> { new ParameterDeclaration(context.popTypeSpecifier(),
                    context.popDeclaration(), context.scope()) });
}

void ContextualSyntaxNodeBuilder::parameterList(AbstractSyntaxTreeBuilderContext& context) {
    context.pushParameterList(std::unique_ptr<ParameterList> { new ParameterList(context.popParameter()) });
}

void ContextualSyntaxNodeBuilder::addParameterToList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto parameterList = context.popParameterList();
    parameterList->addParameterDeclaration(context.popParameter());
    context.pushParameterList(std::move(parameterList));
}

void ContextualSyntaxNodeBuilder::identifierDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclaration(std::unique_ptr<Declaration> { new Identifier(context.popTerminal()) });
}

void ContextualSyntaxNodeBuilder::functionDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushFunctionDeclaration(
            std::unique_ptr<FunctionDeclaration> { new FunctionDeclaration(context.popDeclaration(),
                    context.popParameterList(), context.scope()) });
}

void ContextualSyntaxNodeBuilder::noargFunctionDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushFunctionDeclaration(
            std::unique_ptr<FunctionDeclaration> { new FunctionDeclaration(context.popDeclaration(), context.scope()) });
}

void ContextualSyntaxNodeBuilder::arrayDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDeclaration(
            std::unique_ptr<Declaration> { new ArrayDeclaration(context.popDeclaration(), context.popExpression(),
                    context.scope()) });
}

void ContextualSyntaxNodeBuilder::pointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushPointer(std::unique_ptr<Pointer> { new Pointer() });
}

void ContextualSyntaxNodeBuilder::pointerToPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto pointer = context.popPointer();
    pointer->dereference();
    context.pushPointer(std::move(pointer));
}

void ContextualSyntaxNodeBuilder::singleBlock(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new Block(context.popStatement()) });
}

void ContextualSyntaxNodeBuilder::doubleBlock(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    auto secondSubblock = context.popListCarrier();
    auto firstSubblock = context.popListCarrier();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new Block(std::move(firstSubblock), std::move(secondSubblock)) });
}

void ContextualSyntaxNodeBuilder::pointerToDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    auto declaration = context.popDeclaration();
    declaration->dereference(context.popPointer()->getDereferenceCount());
    context.pushDeclaration(std::move(declaration));
}

void ContextualSyntaxNodeBuilder::declarationList(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationList(std::unique_ptr<DeclarationList> { new DeclarationList(context.popDeclaration()) });
}

void ContextualSyntaxNodeBuilder::addDeclarationToList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationList = context.popDeclarationList();
    declarationList->addDeclaration(context.popDeclaration());
    context.pushDeclarationList(std::move(declarationList));
}

void ContextualSyntaxNodeBuilder::variableDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new VariableDeclaration(context.popTypeSpecifier(),
                    context.popDeclarationList(), context.scope()) });
}

void ContextualSyntaxNodeBuilder::variableDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new VariableDefinition(
                    std::unique_ptr<VariableDeclaration> { new VariableDeclaration(context.popTypeSpecifier(),
                            context.popDeclarationList(), context.scope()) }, context.popExpression(), context.scope(),
                    context.line()) });
}

void ContextualSyntaxNodeBuilder::functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new FunctionDefinition(context.popTypeSpecifier(),
                    context.popFunctionDeclaration(), context.popStatement(), context.scope()) });
}

void ContextualSyntaxNodeBuilder::newListCarrier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushListCarrier(std::unique_ptr<ListCarrier> { new ListCarrier { context.popStatement() } });
}

void ContextualSyntaxNodeBuilder::addToListCarrier(AbstractSyntaxTreeBuilderContext& context) {
    auto listCarrier = context.popListCarrier();
    listCarrier->addChild(context.popStatement());
    context.pushListCarrier(std::move(listCarrier));
}

void ContextualSyntaxNodeBuilder::functionsTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new TranslationUnit(context.popListCarrier()) });
}

void ContextualSyntaxNodeBuilder::variablesFunctionsTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    auto functions = context.popListCarrier();
    auto variables = context.popListCarrier();
    context.pushStatement(
            std::unique_ptr<AbstractSyntaxTreeNode> { new TranslationUnit(std::move(variables), std::move(functions)) });
}

void ContextualSyntaxNodeBuilder::newIntegerType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BasicType::INTEGER, context.popTerminal().value });
}

void ContextualSyntaxNodeBuilder::newCharacterType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BasicType::CHARACTER, context.popTerminal().value });
}

void ContextualSyntaxNodeBuilder::newVoidType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BasicType::VOID, context.popTerminal().value });
}

void ContextualSyntaxNodeBuilder::newFloatType(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushTypeSpecifier( { BasicType::FLOAT, context.popTerminal().value });
}

} /* namespace semantic_analyzer */
