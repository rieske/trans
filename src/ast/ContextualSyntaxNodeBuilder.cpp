#include "ContextualSyntaxNodeBuilder.h"

#include <bits/functional_hash.h>
#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclarator.h"
#include "AssignmentExpression.h"
#include "ArgumentExpressionList.h"
#include "BitwiseExpression.h"
#include "Block.h"
#include "ComparisonExpression.h"
#include "DeclarationList.h"
#include "DeclarationSpecifiers.h"
#include "DereferencedDeclarator.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FormalArgument.h"
#include "FunctionCall.h"
#include "FunctionDeclarator.h"
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
#include "Operator.h"
#include "Pointer.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ReturnStatement.h"
#include "ShiftExpression.h"
#include "StorageSpecifier.h"
#include "types/BaseType.h"
#include "IdentifierExpression.h"
#include "ConstantExpression.h"
#include "TerminalSymbol.h"
#include "TranslationUnit.h"
#include "TypeCast.h"
#include "TypeQualifier.h"
#include "TypeSpecifier.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"
#include "WhileLoopHeader.h"

using std::unique_ptr;

static const std::string UNMATCHED { "<unmatched>" };
static const std::string MATCHED { "<matched>" };
static const std::string STATEMENT { "<stmt>" };
static const std::string STATEMENTS { "<statements>" };
static const std::string VAR_DECLARATION { "<var_decl>" };
static const std::string VAR_DECLARATIONS { "<var_decls>" };
static const std::string FUNCTION_DECLARATIONS { "<func_decls>" };
static const std::string TRANSLATION_UNIT { "<program>" };

static const std::string FORMAL_ARGUMENTS { "<param_list>" };
static const std::string FORMAL_ARGUMENTS_DECLARATION { "<param_type_list>" };
static const std::string TYPE_SPECIFIER { "<type_spec>" };
static const std::string CONSTANT { "<const>" };
static const std::string PRIMARY_EXPRESSION { "<primary_exp>" };
static const std::string UNARY_OPERATOR { "<u_op>" };
static const std::string MULTIPLICATION_OPERATOR { "<m_op>" };
static const std::string ADDITION_OPERATOR { "<add_op>" };
static const std::string SHIFT_OPERATOR { "<s_op>" };
static const std::string COMPARISON_OPERATOR { "<ml_op>" };
static const std::string EQUALITY_OPERATOR { "<eq_op>" };
static const std::string ASSIGNMENT_OPERATOR { "<a_op>" };

namespace ast {

void doNothing(AbstractSyntaxTreeBuilderContext&) {
}

void shortType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "short type is not implemented yet" };
}

void integerType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BaseType::newInteger(), context.popTerminal().value });
}

void longType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "long type is not implemented yet" };
}

void characterType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BaseType::newCharacter(), context.popTerminal().value });
}

void voidType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BaseType::newVoid(), context.popTerminal().value });
}

void floatType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { BaseType::newFloat(), context.popTerminal().value });
}

void doubleType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "double type is not implemented yet" };
}

void signedType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "signed type is not implemented yet" };
}

void unsignedType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "unsigned type is not implemented yet" };
}

void typedefName(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "typedefName type is not implemented yet" };
}

void structOrUnionType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "structOrUnionType type is not implemented yet" };
}

void enumType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "enumType type is not implemented yet" };
}

void parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
}

void declarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popTypeSpecifier() });
}

void addDeclarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto typeSpecifier = context.popTypeSpecifier();
    context.pushDeclarationSpecifiers( { typeSpecifier, declarationSpecifiers });
}

void declarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popStorageSpecifier() });
}

void addDeclarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto storageSpecifier = context.popStorageSpecifier();
    context.pushDeclarationSpecifiers( { storageSpecifier, declarationSpecifiers });
}

void declarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popTypeQualifier() });
}

void addDeclarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto typeQualifier = context.popTypeQualifier();
    context.pushDeclarationSpecifiers( { typeQualifier, declarationSpecifiers });
}

void identifierDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Identifier>(context.popTerminal()));
}

void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDeclarator(std::make_unique<ArrayDeclarator>(context.popDeclarator(), context.popExpression()));
}

void abstractArrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    //context.pushDeclarator(std::make_unique<ArrayDeclarator>(context.popDeclarator()));
    throw std::runtime_error { "abstract array declarator is not implemented yet" };
}

void functionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushFunctionDeclaration(std::make_unique<FunctionDeclarator>(context.popDeclarator(), context.popArgumentsDeclaration().first));
}

void deprecatedFunctionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    // context.pushFunctionDeclaration(std::make_unique<FunctionDeclarator>(context.popDeclarator(), context.popIdentifierList()));
    throw std::runtime_error { "deprecatedFunctionDeclarator is not implemented yet" };
}

void noargFunctionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushFunctionDeclaration(std::make_unique<FunctionDeclarator>(context.popDeclarator()));
}

void pointerToDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<DereferencedDeclarator>(context.popDeclarator()));
}

void parameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers(), context.popDeclarator() });
}

void abstractParameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "abstractParameterDeclaration is not implemented yet" };
    //context.pushFormalArgument(std::make_unique<FormalArgument>(context.popDeclarationSpecifiers(), context.popAbstractDeclarator()));
}

void parameterBaseTypeDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers() });
}

void formalArguments(AbstractSyntaxTreeBuilderContext& context) {
    FormalArguments formalArguments;
    formalArguments.push_back(context.popFormalArgument());
    context.pushFormalArguments(std::move(formalArguments));
}

void addFormalArgument(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto formalArguments = context.popFormalArguments();
    formalArguments.push_back(context.popFormalArgument());
    context.pushFormalArguments(std::move(formalArguments));
}

void formalArgumentsDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushArgumentsDeclaration(std::make_pair(context.popFormalArguments(), false));
}

void formalArgumentsWithVararg(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    //context.pushArgumentsDeclaration(std::make_pair(context.popFormalArguments(), true));
    throw std::runtime_error { "formalArgumentsWithVararg is not implemented yet" };
}

void integerConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, BaseType::INTEGER, constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, BaseType::CHARACTER, constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, BaseType::FLOAT, constant.context });
}

void enumerationConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    throw std::runtime_error { "enumerationConstant is not implemented yet" };
}

void identifierExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto identifier = context.popTerminal();
    context.pushExpression(std::make_unique<IdentifierExpression>(identifier.value, identifier.context));
}

void constantExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<ConstantExpression>(context.popConstant()));
}

void stringExpression(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "stringExpression is not implemented yet" };
}

void arrayAccess(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ]
    context.popTerminal(); // [
    auto subscriptExpression = context.popExpression();
    auto postfixExpression = context.popExpression();
    context.pushExpression(std::make_unique<ArrayAccess>(std::move(postfixExpression), std::move(subscriptExpression)));
}

void functionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(std::make_unique<FunctionCall>(context.popExpression(), context.popAssignmentExpressionList()));
}

void noargFunctionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(std::make_unique<FunctionCall>(context.popExpression(), std::make_unique<ArgumentExpressionList>()));
}

void directMemberAccess(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "directMemberAccess is not implemented yet" };
}

void pointeeMemberAccess(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "pointeeMemberAccess is not implemented yet" };
}

void postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PostfixExpression>(context.popExpression(), std::make_unique<Operator>(context.popTerminal().type)));
}

void prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PrefixExpression>(std::make_unique<Operator>(context.popTerminal().value), context.popExpression()));
}

void unaryExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<UnaryExpression>(std::make_unique<Operator>(context.popTerminal().value), context.popExpression()));
}

void sizeofExpression(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "sizeofExpression is not implemented yet" };
}

void sizeofTypeExpression(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "sizeofTypeExpression is not implemented yet" };
}

void typeCast(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    //context.pushExpression(std::make_unique<TypeCast>(context.popTypeName(), context.popExpression()));
    context.popTerminal(); // (
    throw std::runtime_error { "typeCast is not implemented yet" };
}

void arithmeticExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto arithmeticOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<ArithmeticExpression>(std::move(leftHandSide), std::move(arithmeticOperator), std::move(rightHandSide)));
}

void shiftExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto additionExpression = context.popExpression();
    auto shiftExpression = context.popExpression();
    auto shiftOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<ShiftExpression>(std::move(shiftExpression), std::move(shiftOperator), std::move(additionExpression)));
}

void relationalExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto comparisonOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<ComparisonExpression>(std::move(leftHandSide), std::move(comparisonOperator), std::move(rightHandSide)));
}

ContextualSyntaxNodeBuilder::ContextualSyntaxNodeBuilder() {

    nodeCreatorRegistry[TYPE_SPECIFIER][ { "short" }] = shortType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "int" }] = integerType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "long" }] = longType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "char" }] = characterType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "void" }] = voidType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "float" }] = floatType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "double" }] = doubleType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "signed" }] = signedType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "unsigned" }] = unsignedType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "typedef_name" }] = typedefName;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "<struct_or_union_spec>" }] = structOrUnionType;
    nodeCreatorRegistry[TYPE_SPECIFIER][ { "<enum_spec>" }] = enumType;

    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { TYPE_SPECIFIER }] = declarationTypeSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { TYPE_SPECIFIER, DeclarationSpecifiers::ID }] = addDeclarationTypeSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { "<storage_class_spec>" }] = declarationStorageClassSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { "<storage_class_spec>", DeclarationSpecifiers::ID }] = addDeclarationStorageClassSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { "<type_qualifier>" }] = declarationTypeQualifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { "<type_qualifier>", DeclarationSpecifiers::ID }] = addDeclarationTypeQualifier;

    nodeCreatorRegistry[DirectDeclarator::ID][ { "id" }] = identifierDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { "(", Declarator::ID, ")" }] = parenthesizedExpression;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "[", "<const_exp>", "]" }] = arrayDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "[", "]" }] = abstractArrayDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "(", FORMAL_ARGUMENTS_DECLARATION, ")" }] = functionDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "(", "<id_list>", ")" }] = deprecatedFunctionDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "(", ")" }] = noargFunctionDeclarator;

    nodeCreatorRegistry[Declarator::ID][ { Pointer::ID, DirectDeclarator::ID }] = pointerToDeclarator;
    nodeCreatorRegistry[Declarator::ID][ { DirectDeclarator::ID }] = doNothing;

    nodeCreatorRegistry[FormalArgument::ID][ { DeclarationSpecifiers::ID, Declarator::ID }] = parameterDeclaration;
    nodeCreatorRegistry[FormalArgument::ID][ { DeclarationSpecifiers::ID, "<abstract_declarator>" }] = abstractParameterDeclaration;
    nodeCreatorRegistry[FormalArgument::ID][ { DeclarationSpecifiers::ID }] = parameterBaseTypeDeclaration;

    nodeCreatorRegistry[FORMAL_ARGUMENTS][ { FormalArgument::ID }] = formalArguments;
    nodeCreatorRegistry[FORMAL_ARGUMENTS][ { FORMAL_ARGUMENTS, ",", FormalArgument::ID }] = addFormalArgument;

    nodeCreatorRegistry[FORMAL_ARGUMENTS_DECLARATION][ { FORMAL_ARGUMENTS }] = formalArgumentsDeclaration;
    nodeCreatorRegistry[FORMAL_ARGUMENTS_DECLARATION][ { FORMAL_ARGUMENTS, ",", "..." }] = formalArgumentsWithVararg;

    nodeCreatorRegistry[CONSTANT][ { "int_const" }] = integerConstant;
    nodeCreatorRegistry[CONSTANT][ { "char_const" }] = characterConstant;
    nodeCreatorRegistry[CONSTANT][ { "float_const" }] = floatConstant;
    nodeCreatorRegistry[CONSTANT][ { "enumeration_const" }] = enumerationConstant;

    nodeCreatorRegistry[PRIMARY_EXPRESSION][ { "id" }] = identifierExpression;
    nodeCreatorRegistry[PRIMARY_EXPRESSION][ { CONSTANT }] = constantExpression;
    nodeCreatorRegistry[PRIMARY_EXPRESSION][ { "string" }] = stringExpression;
    nodeCreatorRegistry[PRIMARY_EXPRESSION][ { "(", Expression::ID, ")" }] = parenthesizedExpression;

    nodeCreatorRegistry[PostfixExpression::ID][ { PRIMARY_EXPRESSION }] = doNothing;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "[", Expression::ID, "]" }] = arrayAccess;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "(", ArgumentExpressionList::ID, ")" }] = functionCall;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "(", ")" }] = noargFunctionCall;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "." }] = directMemberAccess;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "->" }] = pointeeMemberAccess;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "++" }] = postfixIncrementDecrement;
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "--" }] = postfixIncrementDecrement;

    nodeCreatorRegistry[UnaryExpression::ID][ { PostfixExpression::ID }] = doNothing;
    nodeCreatorRegistry[UnaryExpression::ID][ { "++", UnaryExpression::ID }] = prefixIncrementDecrement;
    nodeCreatorRegistry[UnaryExpression::ID][ { "--", UnaryExpression::ID }] = prefixIncrementDecrement;
    nodeCreatorRegistry[UnaryExpression::ID][ { "<unary_operator>", TypeCast::ID }] = unaryExpression;
    nodeCreatorRegistry[UnaryExpression::ID][ { "sizeof", UnaryExpression::ID }] = sizeofExpression;
    nodeCreatorRegistry[UnaryExpression::ID][ { "sizeof", "(", "<type_name>", ")" }] = sizeofTypeExpression;

    nodeCreatorRegistry[TypeCast::ID][ { UnaryExpression::ID }] = doNothing;
    nodeCreatorRegistry[TypeCast::ID][ { "(", "<type_name>", ")", TypeCast::ID }] = typeCast;

    nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][ { TypeCast::ID }] = doNothing;
    nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][ { ArithmeticExpression::MULTIPLICATION, "*", TypeCast::ID }] = arithmeticExpression;
    nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][ { ArithmeticExpression::MULTIPLICATION, "/", TypeCast::ID }] = arithmeticExpression;
    nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][ { ArithmeticExpression::MULTIPLICATION, "%", TypeCast::ID }] = arithmeticExpression;

    nodeCreatorRegistry[ArithmeticExpression::ADDITION][ { ArithmeticExpression::MULTIPLICATION }] = doNothing;
    nodeCreatorRegistry[ArithmeticExpression::ADDITION][ { ArithmeticExpression::ADDITION, "+", ArithmeticExpression::MULTIPLICATION }] = arithmeticExpression;
    nodeCreatorRegistry[ArithmeticExpression::ADDITION][ { ArithmeticExpression::ADDITION, "-", ArithmeticExpression::MULTIPLICATION }] = arithmeticExpression;

    nodeCreatorRegistry[ShiftExpression::ID][ { ArithmeticExpression::ADDITION }] = doNothing;
    nodeCreatorRegistry[ShiftExpression::ID][ { ShiftExpression::ID, "<<", ArithmeticExpression::ADDITION }] = shiftExpression;
    nodeCreatorRegistry[ShiftExpression::ID][ { ShiftExpression::ID, ">>", ArithmeticExpression::ADDITION }] = shiftExpression;

    nodeCreatorRegistry[ComparisonExpression::RELATIONAL][ { ShiftExpression::ID }] = doNothing;
    nodeCreatorRegistry[ComparisonExpression::RELATIONAL][ { ComparisonExpression::RELATIONAL, "<", ShiftExpression::ID }] = relationalExpression;
    nodeCreatorRegistry[ComparisonExpression::RELATIONAL][ { ComparisonExpression::RELATIONAL, ">", ShiftExpression::ID }] = relationalExpression;
    nodeCreatorRegistry[ComparisonExpression::RELATIONAL][ { ComparisonExpression::RELATIONAL, "<=", ShiftExpression::ID }] = relationalExpression;
    nodeCreatorRegistry[ComparisonExpression::RELATIONAL][ { ComparisonExpression::RELATIONAL, ">=", ShiftExpression::ID }] = relationalExpression;

    nodeCreatorRegistry[ComparisonExpression::EQUALITY][ { ComparisonExpression::RELATIONAL }] = doNothing;
    nodeCreatorRegistry[ComparisonExpression::EQUALITY][ { ComparisonExpression::EQUALITY, "==", ComparisonExpression::RELATIONAL }] = relationalExpression;
    nodeCreatorRegistry[ComparisonExpression::EQUALITY][ { ComparisonExpression::EQUALITY, "!=", ComparisonExpression::RELATIONAL }] = relationalExpression;

    /*
     nodeCreatorRegistry[BitwiseExpression::AND][ { BitwiseExpression::ID, "&", ComparisonExpression::EQUALITY }] =
     ContextualSyntaxNodeBuilder::bitwiseExpression;
     nodeCreatorRegistry[BitwiseExpression::AND][ { ComparisonExpression::EQUALITY }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[BitwiseExpression::XOR][ { BitwiseExpression::XOR, "^", BitwiseExpression::AND }] =
     ContextualSyntaxNodeBuilder::bitwiseExpression;
     nodeCreatorRegistry[BitwiseExpression::XOR][ { BitwiseExpression::AND }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[BitwiseExpression::OR][ { BitwiseExpression::OR, "|", BitwiseExpression::XOR }] =
     ContextualSyntaxNodeBuilder::bitwiseExpression;
     nodeCreatorRegistry[BitwiseExpression::OR][ { BitwiseExpression::XOR }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[LogicalAndExpression::ID][ { LogicalAndExpression::ID, "&&", BitwiseExpression::OR }] =
     ContextualSyntaxNodeBuilder::logicalAndExpression;
     nodeCreatorRegistry[LogicalAndExpression::ID][ { BitwiseExpression::OR }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[LogicalOrExpression::ID][ { LogicalOrExpression::ID, "||", Expression::ID }] = ContextualSyntaxNodeBuilder::logicalOrExpression;
     nodeCreatorRegistry[LogicalOrExpression::ID][ { LogicalAndExpression::ID }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[AssignmentExpression::ID][ { UnaryExpression::ID, "<a_op>", AssignmentExpression::ID }] =
     ContextualSyntaxNodeBuilder::assignmentExpression;
     nodeCreatorRegistry[AssignmentExpression::ID][ { LogicalOrExpression::ID }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[AssignmentExpressionList::ID][ { AssignmentExpression::ID }] = ContextualSyntaxNodeBuilder::createAssignmentExpressionList;
     nodeCreatorRegistry[AssignmentExpressionList::ID][ { AssignmentExpressionList::ID, ",", AssignmentExpression::ID }] =
     ContextualSyntaxNodeBuilder::addAssignmentExpressionToList;

     nodeCreatorRegistry[Expression::ID][ { Expression::ID, ",", AssignmentExpression::ID }] = ContextualSyntaxNodeBuilder::expressionList;
     nodeCreatorRegistry[Expression::ID][ { AssignmentExpression::ID }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[JumpStatement::ID][ { "continue", ";" }] = ContextualSyntaxNodeBuilder::loopJumpStatement;
     nodeCreatorRegistry[JumpStatement::ID][ { "break", ";" }] = ContextualSyntaxNodeBuilder::loopJumpStatement;
     nodeCreatorRegistry[JumpStatement::ID][ { "return", Expression::ID, ";" }] = ContextualSyntaxNodeBuilder::returnStatement;

     nodeCreatorRegistry[IOStatement::ID][ { "output", Expression::ID, ";" }] = ContextualSyntaxNodeBuilder::inputOutputStatement;
     nodeCreatorRegistry[IOStatement::ID][ { "input", Expression::ID, ";" }] = ContextualSyntaxNodeBuilder::inputOutputStatement;

     nodeCreatorRegistry[LoopHeader::ID][ { "while", "(", Expression::ID, ")" }] = ContextualSyntaxNodeBuilder::whileLoopHeader;
     nodeCreatorRegistry[LoopHeader::ID][ { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")" }] =
     ContextualSyntaxNodeBuilder::forLoopHeader;

     nodeCreatorRegistry[UNMATCHED][ { "if", "(", Expression::ID, ")", STATEMENT }] = ContextualSyntaxNodeBuilder::ifStatement;
     nodeCreatorRegistry[UNMATCHED][ { "if", "(", Expression::ID, ")", MATCHED, "else", UNMATCHED }] = ContextualSyntaxNodeBuilder::ifElseStatement;
     nodeCreatorRegistry[UNMATCHED][ { LoopHeader::ID, UNMATCHED }] = ContextualSyntaxNodeBuilder::loopStatement;

     nodeCreatorRegistry[MATCHED][ { Expression::ID, ";" }] = ContextualSyntaxNodeBuilder::expressionStatement;
     nodeCreatorRegistry[MATCHED][ { ";" }] = ContextualSyntaxNodeBuilder::emptyStatement;
     nodeCreatorRegistry[MATCHED][ { IOStatement::ID }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[MATCHED][ { Block::ID }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[MATCHED][ { JumpStatement::ID }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[MATCHED][ { "if", "(", Expression::ID, ")", MATCHED, "else", MATCHED }] = ContextualSyntaxNodeBuilder::ifElseStatement;
     nodeCreatorRegistry[MATCHED][ { LoopHeader::ID, MATCHED }] = ContextualSyntaxNodeBuilder::loopStatement;

     nodeCreatorRegistry[FORMAL_ARGUMENTS][ { FormalArgument::ID }] = ContextualSyntaxNodeBuilder::parameterList;
     nodeCreatorRegistry[FORMAL_ARGUMENTS][ { FORMAL_ARGUMENTS, ",", FormalArgument::ID }] = ContextualSyntaxNodeBuilder::addParameterToList;

     nodeCreatorRegistry[Pointer::ID][ { "*" }] = ContextualSyntaxNodeBuilder::pointer;
     nodeCreatorRegistry[Pointer::ID][ { Pointer::ID, "*" }] = ContextualSyntaxNodeBuilder::pointerToPointer;

     nodeCreatorRegistry[Block::ID][ { "{", VAR_DECLARATIONS, STATEMENTS, "}" }] = ContextualSyntaxNodeBuilder::doubleBlock;
     nodeCreatorRegistry[Block::ID][ { "{", VAR_DECLARATIONS, "}" }] = ContextualSyntaxNodeBuilder::singleBlock;
     nodeCreatorRegistry[Block::ID][ { "{", STATEMENTS, "}" }] = ContextualSyntaxNodeBuilder::singleBlock;
     nodeCreatorRegistry[Block::ID][ { "{", "}" }] = ContextualSyntaxNodeBuilder::parenthesizedExpression;

     nodeCreatorRegistry[DeclarationList::ID][ { DirectDeclarator::ID }] = ContextualSyntaxNodeBuilder::declarationList;
     nodeCreatorRegistry[DeclarationList::ID][ { DeclarationList::ID, ",", DirectDeclarator::ID }] = ContextualSyntaxNodeBuilder::addDeclarationToList;

     nodeCreatorRegistry[VariableDeclaration::ID][ { TYPE_SPECIFIER, DeclarationList::ID, ";" }] = ContextualSyntaxNodeBuilder::variableDeclaration;
     nodeCreatorRegistry[VariableDeclaration::ID][ { TYPE_SPECIFIER, DeclarationList::ID, "=", AssignmentExpression::ID, ";" }] =
     ContextualSyntaxNodeBuilder::variableDefinition;

     nodeCreatorRegistry[FunctionDefinition::ID][ { TYPE_SPECIFIER, DirectDeclarator::ID, Block::ID }] = ContextualSyntaxNodeBuilder::functionDefinition;

     nodeCreatorRegistry[STATEMENTS][ { STATEMENT }] = ContextualSyntaxNodeBuilder::newListCarrier;
     nodeCreatorRegistry[STATEMENTS][ { STATEMENTS, STATEMENT }] = ContextualSyntaxNodeBuilder::addToListCarrier;

     nodeCreatorRegistry[VAR_DECLARATIONS][ { VAR_DECLARATION }] = ContextualSyntaxNodeBuilder::newListCarrier;
     nodeCreatorRegistry[VAR_DECLARATIONS][ { VAR_DECLARATIONS, VAR_DECLARATION }] = ContextualSyntaxNodeBuilder::addToListCarrier;

     nodeCreatorRegistry[FUNCTION_DECLARATIONS][ { FunctionDefinition::ID }] = ContextualSyntaxNodeBuilder::newListCarrier;
     nodeCreatorRegistry[FUNCTION_DECLARATIONS][ { FUNCTION_DECLARATIONS, FunctionDefinition::ID }] = ContextualSyntaxNodeBuilder::addToListCarrier;

     nodeCreatorRegistry[TRANSLATION_UNIT][ { FUNCTION_DECLARATIONS }] = ContextualSyntaxNodeBuilder::functionsTranslationUnit;
     nodeCreatorRegistry[TRANSLATION_UNIT][ { VAR_DECLARATIONS, FUNCTION_DECLARATIONS }] =
     ContextualSyntaxNodeBuilder::variablesFunctionsTranslationUnit;

     nodeCreatorRegistry[STATEMENT][ { MATCHED }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[STATEMENT][ { UNMATCHED }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[UNARY_OPERATOR][ { "&" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[UNARY_OPERATOR][ { "*" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[UNARY_OPERATOR][ { "+" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[UNARY_OPERATOR][ { "-" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[UNARY_OPERATOR][ { "!" }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[MULTIPLICATION_OPERATOR][ { "*" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[MULTIPLICATION_OPERATOR][ { "/" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[MULTIPLICATION_OPERATOR][ { "%" }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[ADDITION_OPERATOR][ { "+" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ADDITION_OPERATOR][ { "-" }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[SHIFT_OPERATOR][ { ">>" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[SHIFT_OPERATOR][ { "<<" }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[COMPARISON_OPERATOR][ { "<" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[COMPARISON_OPERATOR][ { ">" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[COMPARISON_OPERATOR][ { "<=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[COMPARISON_OPERATOR][ { ">=" }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[EQUALITY_OPERATOR][ { "==" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[EQUALITY_OPERATOR][ { "!=" }] = ContextualSyntaxNodeBuilder::doNothing;

     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "+=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "-=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "*=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "/=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "%=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "&=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "^=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "|=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { "<<=" }] = ContextualSyntaxNodeBuilder::doNothing;
     nodeCreatorRegistry[ASSIGNMENT_OPERATOR][ { ">>=" }] = ContextualSyntaxNodeBuilder::doNothing;*/

}

ContextualSyntaxNodeBuilder::~ContextualSyntaxNodeBuilder() {
}

void ContextualSyntaxNodeBuilder::updateContext(std::string definingSymbol, const std::vector<std::string>& production,
        AbstractSyntaxTreeBuilderContext& context) const
        {
    try {
        nodeCreatorRegistry.at(definingSymbol).at(production)(context);
    } catch (std::out_of_range& exception) {
        noCreatorDefined(definingSymbol, production);
    }
}

void ContextualSyntaxNodeBuilder::noCreatorDefined(std::string definingSymbol, const std::vector<std::string>& production) {
    std::ostringstream productionString;
    for (auto& symbol : production) {
        productionString << symbol << " ";
    }
    throw std::runtime_error { "no AST creator defined for production `" + definingSymbol + " ::= " + productionString.str() + "`" };
}

void ContextualSyntaxNodeBuilder::bitwiseExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto bitwiseOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<BitwiseExpression>(std::move(leftHandSide), std::move(bitwiseOperator), std::move(rightHandSide)));
}

void ContextualSyntaxNodeBuilder::logicalAndExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<LogicalAndExpression>(std::move(leftHandSide), std::move(rightHandSide)));
}

void ContextualSyntaxNodeBuilder::logicalOrExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<LogicalOrExpression>(std::move(leftHandSide), std::move(rightHandSide)));
}

void ContextualSyntaxNodeBuilder::assignmentExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto assignmentOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(
            std::make_unique<AssignmentExpression>(std::move(leftHandSide), std::move(assignmentOperator), std::move(rightHandSide)));
}

void ContextualSyntaxNodeBuilder::createAssignmentExpressionList(AbstractSyntaxTreeBuilderContext& context) {
    context.pushAssignmentExpressionList(std::make_unique<ArgumentExpressionList>(context.popExpression()));
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
    context.pushExpression(std::make_unique<ExpressionList>(std::move(leftHandSide), std::move(rightHandSide)));
}

void ContextualSyntaxNodeBuilder::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::make_unique<JumpStatement>(context.popTerminal()));
    context.popTerminal();
}

void ContextualSyntaxNodeBuilder::returnStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<ReturnStatement>(context.popExpression()));
}

void ContextualSyntaxNodeBuilder::inputOutputStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(std::make_unique<IOStatement>(context.popTerminal(), context.popExpression()));
}

void ContextualSyntaxNodeBuilder::whileLoopHeader(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushLoopHeader(std::make_unique<WhileLoopHeader>(context.popExpression()));
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
    context.pushLoopHeader(std::make_unique<ForLoopHeader>(std::move(initialization), std::move(clause), std::move(increment)));
}

void ContextualSyntaxNodeBuilder::ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<IfStatement>(context.popExpression(), context.popStatement()));
}

void ContextualSyntaxNodeBuilder::ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto falsyStatement = context.popStatement();
    auto truthyStatement = context.popStatement();
    context.pushStatement(std::make_unique<IfElseStatement>(context.popExpression(), std::move(truthyStatement), std::move(falsyStatement)));
}

void ContextualSyntaxNodeBuilder::loopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::make_unique<LoopStatement>(context.popLoopHeader(), context.popStatement()));
}

void ContextualSyntaxNodeBuilder::expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(context.popExpression());
}

void ContextualSyntaxNodeBuilder::emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
}

void ContextualSyntaxNodeBuilder::pointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushPointer(std::make_unique<Pointer>());
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
    context.pushStatement(std::make_unique<Block>(context.popStatement()));
}

void ContextualSyntaxNodeBuilder::doubleBlock(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    auto secondSubblock = context.popListCarrier();
    auto firstSubblock = context.popListCarrier();
    context.pushStatement(std::make_unique<Block>(std::move(firstSubblock), std::move(secondSubblock)));
}

void ContextualSyntaxNodeBuilder::declarationList(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationList(std::make_unique<DeclarationList>(context.popDeclarator()));
}

void ContextualSyntaxNodeBuilder::addDeclarationToList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationList = context.popDeclarationList();
    declarationList->addDeclaration(context.popDeclarator());
    context.pushDeclarationList(std::move(declarationList));
}

void ContextualSyntaxNodeBuilder::variableDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(std::make_unique<VariableDeclaration>(context.popTypeSpecifier(), context.popDeclarationList()));
}

void ContextualSyntaxNodeBuilder::variableDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(
            std::make_unique<VariableDefinition>(
                    std::make_unique<VariableDeclaration>(context.popTypeSpecifier(), context.popDeclarationList()),
                    context.popExpression()));
}

void ContextualSyntaxNodeBuilder::functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(
            std::make_unique<FunctionDefinition>(context.popTypeSpecifier(), context.popFunctionDeclaration(), context.popStatement()));
}

void ContextualSyntaxNodeBuilder::newListCarrier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushListCarrier(std::make_unique<ListCarrier>(context.popStatement()));
}

void ContextualSyntaxNodeBuilder::addToListCarrier(AbstractSyntaxTreeBuilderContext& context) {
    auto listCarrier = context.popListCarrier();
    listCarrier->addChild(context.popStatement());
    context.pushListCarrier(std::move(listCarrier));
}

void ContextualSyntaxNodeBuilder::functionsTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::make_unique<TranslationUnit>(context.popListCarrier()));
}

void ContextualSyntaxNodeBuilder::variablesFunctionsTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    auto functions = context.popListCarrier();
    auto variables = context.popListCarrier();
    context.pushStatement(std::make_unique<TranslationUnit>(std::move(variables), std::move(functions)));
}

} /* namespace ast */
