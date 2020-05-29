#include "ContextualSyntaxNodeBuilder.h"

#include <algorithm>
#include <sstream>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclarator.h"
#include "AssignmentExpression.h"
#include "BitwiseExpression.h"
#include "Block.h"
#include "ComparisonExpression.h"
#include "ConstantExpression.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FunctionCall.h"
#include "FunctionDefinition.h"
#include "Identifier.h"
#include "IdentifierExpression.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "IOStatement.h"
#include "JumpStatement.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "Operator.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ReturnStatement.h"
#include "VoidReturnStatement.h"
#include "ShiftExpression.h"
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "WhileLoopHeader.h"

#include "types/VoidType.h"
#include "types/FloatingType.h"

using std::unique_ptr;

static const std::string UNMATCHED { "<unmatched>" };
static const std::string MATCHED { "<matched>" };
static const std::string STATEMENT { "<stat>" };
static const std::string STATEMENTS { "<stat_list>" };
static const std::string DECLARATIONS { "<decl_list>" };
static const std::string EXTERNAL_DECLARATION { "<external_decl>" };
static const std::string TRANSLATION_UNIT { "<translation_unit>" };

static const std::string FORMAL_ARGUMENTS { "<param_list>" };
static const std::string FORMAL_ARGUMENTS_DECLARATION { "<param_type_list>" };
static const std::string ACTUAL_ARGUMENTS { "<argument_exp_list>" };
static const std::string TYPE_SPECIFIER { "<type_spec>" };
static const std::string TYPE_QUALIFIER { "<type_qualifier>" };
static const std::string TYPE_QUALIFIER_LIST { "<type_qualifier_list>" };
static const std::string CONSTANT { "<const>" };
static const std::string PRIMARY_EXPRESSION { "<primary_exp>" };

static const std::string ITERATION_STATEMENT_MATCHED = { "<iteration_stat_matched>" };
static const std::string ITERATION_STATEMENT_UNMATCHED = { "<iteration_stat_unmatched>" };

namespace ast {

void doNothing(AbstractSyntaxTreeBuilderContext&) {
}

void shortType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "short type is not implemented yet" };
}

void integerType(AbstractSyntaxTreeBuilderContext& context) {
    IntegralType integerType { IntegralType::Integral::SIGNED_INT };
    context.pushTypeSpecifier( { integerType, context.popTerminal().value });
}

void longType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "long type is not implemented yet" };
}

void characterType(AbstractSyntaxTreeBuilderContext& context) {
    IntegralType characterType { IntegralType::Integral::SIGNED_CHAR };
    context.pushTypeSpecifier( { characterType, context.popTerminal().value });
}

void voidType(AbstractSyntaxTreeBuilderContext& context) {
    VoidType voidType;
    context.pushTypeSpecifier( { voidType, context.popTerminal().value });
}

void floatType(AbstractSyntaxTreeBuilderContext& context) {
    FloatingType floatType { FloatingType::Floating::FLOAT };
    context.pushTypeSpecifier( { floatType, context.popTerminal().value });
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

void constQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeQualifier(TypeQualifier::CONST);
}

void volatileQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeQualifier(TypeQualifier::VOLATILE);
}

void typeQualifierList(AbstractSyntaxTreeBuilderContext& context) {
    context.newTypeQualifierList(context.popTypeQualifier());
}

void addTypeQualifierToList(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTypeQualifierList(context.popTypeQualifier());
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
    context.pushDirectDeclarator(std::make_unique<Identifier>(context.popTerminal()));
}

void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator(), context.popExpression()));
}

void abstractArrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    //context.pushDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator()));
    throw std::runtime_error { "abstract array declarator is not implemented yet" };
}

void functionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(context.popDirectDeclarator(), context.popArgumentsDeclaration().first));
}

void deprecatedFunctionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    // context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(context.popDirectDeclarator(), context.popIdentifierList()));
    throw std::runtime_error { "deprecatedFunctionDeclarator is not implemented yet" };
}

void noargFunctionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(context.popDirectDeclarator()));
}

void pointerToDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(context.popDirectDeclarator(), context.popPointers()));
}

void declarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(context.popDirectDeclarator()));
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
    context.pushConstant( { constant.value, IntegralType { IntegralType::Integral::SIGNED_INT }, constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, IntegralType { IntegralType::Integral::SIGNED_CHAR }, constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    throw std::runtime_error { "floating constants not implemented yet" };
//    /context.pushConstant( { constant.value, BaseType::FLOAT, constant.context });
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
    context.pushExpression(std::make_unique<FunctionCall>(context.popExpression(), context.popActualArgumentsList()));
}

void noargFunctionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(std::make_unique<FunctionCall>(context.popExpression()));
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

void bitwiseExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto bitwiseOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<BitwiseExpression>(std::move(leftHandSide), std::move(bitwiseOperator), std::move(rightHandSide)));
}

void logicalAndExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<LogicalAndExpression>(std::move(leftHandSide), std::move(rightHandSide)));
}

void logicalOrExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<LogicalOrExpression>(std::move(leftHandSide), std::move(rightHandSide)));
}

void conditionalExpression(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "conditionalExpression is not implemented yet" };
}

void assignmentExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto assignmentOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(
            std::make_unique<AssignmentExpression>(std::move(leftHandSide), std::move(assignmentOperator), std::move(rightHandSide)));
}

void initializer(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "initializer is not implemented yet" };
}

void initializedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator)));
}

void initializedDeclaratorWithInitializer(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    auto initializerExpression = context.popExpression();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator), std::move(initializerExpression)));
}

void initializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<std::unique_ptr<InitializedDeclarator>> declarators;
    declarators.push_back(context.popInitializedDeclarator());
    context.pushInitializedDeclarators(std::move(declarators));
}

void addToInitializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto initializedDeclarators = context.popInitializedDeclarators();
    initializedDeclarators.push_back(context.popInitializedDeclarator());
    context.pushInitializedDeclarators(std::move(initializedDeclarators));
}

void initializedDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto initializedDeclarators = context.popInitializedDeclarators();
    context.pushDeclaration(std::make_unique<Declaration>(declarationSpecifiers, std::move(initializedDeclarators)));
}

void declaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    context.pushDeclaration(std::make_unique<Declaration>(declarationSpecifiers));
}

void declarationList(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<std::unique_ptr<Declaration>> declarations;
    declarations.push_back(context.popDeclaration());
    context.pushDeclarationList(std::move(declarations));
}

void addDeclarationToList(AbstractSyntaxTreeBuilderContext& context) {
    auto declarations = context.popDeclarationList();
    declarations.push_back(context.popDeclaration());
    context.pushDeclarationList(std::move(declarations));
}

void expressionList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<ExpressionList>(std::move(leftHandSide), std::move(rightHandSide)));
}

void inputOutputStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(std::make_unique<IOStatement>(context.popTerminal(), context.popExpression()));
}

void pointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.newPointer(Pointer { });
}

void pointerToPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pointerToPointer(Pointer { });
}

void qualifiedPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.newPointer(context.popTypeQualifierList());
}

void qualifiedPointerToPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pointerToPointer( { context.popTypeQualifierList() });
}

void ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto falsyStatement = context.popStatement();
    auto truthyStatement = context.popStatement();
    context.pushStatement(std::make_unique<IfElseStatement>(context.popExpression(), std::move(truthyStatement), std::move(falsyStatement)));
}

void statementList(AbstractSyntaxTreeBuilderContext& context) {
    context.newStatementList(context.popStatement());
}

void addToStatementList(AbstractSyntaxTreeBuilderContext& context) {
    context.addToStatementList(context.popStatement());
}

void returnExpressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<ReturnStatement>(context.popExpression()));
}

void returnVoidStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<VoidReturnStatement>());
}

void createActualArgumentsList(AbstractSyntaxTreeBuilderContext& context) {
    context.newActualArgumentsList(context.popExpression());
}

void addToActualArgumentsList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.addToActualArgumentsList(context.popExpression());
}

void declarationCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<Block>(context.popDeclarationList(), std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> { }));
}

void statementCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<Block>(std::vector<std::unique_ptr<Declaration>> { }, context.popStatementList()));
}

void fullCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    auto declarations = context.popDeclarationList();
    auto statements = context.popStatementList();
    context.pushStatement(std::make_unique<Block>(std::move(declarations), std::move(statements)));
}

void emptyCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<Block>(
                std::vector<std::unique_ptr<Declaration>> { },
                std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>{}));
}

void expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(context.popExpression());
}

void emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
}

void functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto declarator = context.popDeclarator();
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<FunctionDefinition>(std::move(declarationSpecifiers), std::move(declarator), std::move(statement)));
}

void defaultReturnTypeFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    IntegralType type { IntegralType::Integral::SIGNED_INT };
    DeclarationSpecifiers defaultReturnTypeSpecifiers { TypeSpecifier { type, "int" } };
    context.pushStatement(std::make_unique<FunctionDefinition>(defaultReturnTypeSpecifiers, context.popDeclarator(), context.popStatement()));
}

void externalFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(context.popStatement());
}

void externalDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(context.popDeclaration());
}

void translationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}

void addToTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
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

    nodeCreatorRegistry[TYPE_QUALIFIER][ { "const" }] = constQualifier;
    nodeCreatorRegistry[TYPE_QUALIFIER][ { "volatile" }] = volatileQualifier;

    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { TYPE_SPECIFIER }] = declarationTypeSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { TYPE_SPECIFIER, DeclarationSpecifiers::ID }] = addDeclarationTypeSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { "<storage_class_spec>" }] = declarationStorageClassSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { "<storage_class_spec>", DeclarationSpecifiers::ID }] = addDeclarationStorageClassSpecifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { TYPE_QUALIFIER }] = declarationTypeQualifier;
    nodeCreatorRegistry[DeclarationSpecifiers::ID][ { TYPE_QUALIFIER, DeclarationSpecifiers::ID }] = addDeclarationTypeQualifier;

    nodeCreatorRegistry[DirectDeclarator::ID][ { "id" }] = identifierDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { "(", Declarator::ID, ")" }] = parenthesizedExpression;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "[", "<const_exp>", "]" }] = arrayDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "[", "]" }] = abstractArrayDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "(", FORMAL_ARGUMENTS_DECLARATION, ")" }] = functionDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "(", "<id_list>", ")" }] = deprecatedFunctionDeclarator;
    nodeCreatorRegistry[DirectDeclarator::ID][ { DirectDeclarator::ID, "(", ")" }] = noargFunctionDeclarator;

    nodeCreatorRegistry[Declarator::ID][ { Pointer::ID, DirectDeclarator::ID }] = pointerToDeclarator;
    nodeCreatorRegistry[Declarator::ID][ { DirectDeclarator::ID }] = declarator;

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
    nodeCreatorRegistry[PostfixExpression::ID][ { PostfixExpression::ID, "(", ACTUAL_ARGUMENTS, ")" }] = functionCall;
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

    nodeCreatorRegistry[BitwiseExpression::AND][ { ComparisonExpression::EQUALITY }] = doNothing;
    nodeCreatorRegistry[BitwiseExpression::AND][ { BitwiseExpression::AND, "&", ComparisonExpression::EQUALITY }] = bitwiseExpression;

    nodeCreatorRegistry[BitwiseExpression::XOR][ { BitwiseExpression::AND }] = doNothing;
    nodeCreatorRegistry[BitwiseExpression::XOR][ { BitwiseExpression::XOR, "^", BitwiseExpression::AND }] = bitwiseExpression;

    nodeCreatorRegistry[BitwiseExpression::OR][ { BitwiseExpression::XOR }] = doNothing;
    nodeCreatorRegistry[BitwiseExpression::OR][ { BitwiseExpression::OR, "|", BitwiseExpression::XOR }] = bitwiseExpression;

    nodeCreatorRegistry[LogicalAndExpression::ID][ { BitwiseExpression::OR }] = doNothing;
    nodeCreatorRegistry[LogicalAndExpression::ID][ { LogicalAndExpression::ID, "&&", BitwiseExpression::OR }] = logicalAndExpression;

    nodeCreatorRegistry[LogicalOrExpression::ID][ { LogicalAndExpression::ID }] = doNothing;
    nodeCreatorRegistry[LogicalOrExpression::ID][ { LogicalOrExpression::ID, "||", LogicalAndExpression::ID }] = logicalOrExpression;

    nodeCreatorRegistry["<conditional_exp>"][ { LogicalOrExpression::ID }] = doNothing;
    nodeCreatorRegistry["<conditional_exp>"][ { LogicalOrExpression::ID, "?", "<exp>", ":", "<conditional_exp>" }] = conditionalExpression;

    nodeCreatorRegistry[AssignmentExpression::ID][ { "<conditional_exp>" }] = doNothing;
    nodeCreatorRegistry[AssignmentExpression::ID][ { UnaryExpression::ID, "<assignment_operator>", AssignmentExpression::ID }] = assignmentExpression;

    nodeCreatorRegistry["<initializer>"][ { AssignmentExpression::ID }] = doNothing;
    nodeCreatorRegistry["<initializer>"][ { "{", "<initializer_list>", "}" }] = initializer;

    nodeCreatorRegistry[InitializedDeclarator::ID][ { Declarator::ID }] = initializedDeclarator;
    nodeCreatorRegistry[InitializedDeclarator::ID][ { Declarator::ID, "=", "<initializer>" }] = initializedDeclaratorWithInitializer;

    nodeCreatorRegistry["<init_declarator_list>"][ { InitializedDeclarator::ID }] = initializedDeclaratorList;
    nodeCreatorRegistry["<init_declarator_list>"][ { "<init_declarator_list>", ",", InitializedDeclarator::ID }] = addToInitializedDeclaratorList;

    nodeCreatorRegistry[Declaration::ID][ { DeclarationSpecifiers::ID, "<init_declarator_list>", ";" }] = initializedDeclaration;
    nodeCreatorRegistry[Declaration::ID][ { DeclarationSpecifiers::ID, ";" }] = declaration;

    nodeCreatorRegistry[DECLARATIONS][ { Declaration::ID }] = declarationList;
    nodeCreatorRegistry[DECLARATIONS][ { DECLARATIONS, Declaration::ID }] = addDeclarationToList;

    nodeCreatorRegistry[Expression::ID][ { AssignmentExpression::ID }] = doNothing;
    nodeCreatorRegistry[Expression::ID][ { Expression::ID, ",", AssignmentExpression::ID }] = expressionList;

    nodeCreatorRegistry[TYPE_QUALIFIER_LIST][ { TYPE_QUALIFIER }] = typeQualifierList;
    nodeCreatorRegistry[TYPE_QUALIFIER_LIST][ { TYPE_QUALIFIER_LIST, TYPE_QUALIFIER }] = addTypeQualifierToList;

    nodeCreatorRegistry[Pointer::ID][ { "*", TYPE_QUALIFIER_LIST }] = qualifiedPointer;
    nodeCreatorRegistry[Pointer::ID][ { "*" }] = pointer;
    nodeCreatorRegistry[Pointer::ID][ { "*", TYPE_QUALIFIER_LIST, Pointer::ID }] = qualifiedPointerToPointer;
    nodeCreatorRegistry[Pointer::ID][ { "*", Pointer::ID }] = pointerToPointer;

    // FIXME: probably better to create enums for each operator in context
    nodeCreatorRegistry["<unary_operator>"][ { "&" }] = doNothing;
    nodeCreatorRegistry["<unary_operator>"][ { "*" }] = doNothing;
    nodeCreatorRegistry["<unary_operator>"][ { "+" }] = doNothing;
    nodeCreatorRegistry["<unary_operator>"][ { "-" }] = doNothing;
    nodeCreatorRegistry["<unary_operator>"][ { "~" }] = doNothing;
    nodeCreatorRegistry["<unary_operator>"][ { "!" }] = doNothing;

    // FIXME: probably better to create enums for each operator in context
    nodeCreatorRegistry["<assignment_operator>"][ { "=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "*=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "/=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "%=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "+=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "-=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "<<=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { ">>=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "&=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "^=" }] = doNothing;
    nodeCreatorRegistry["<assignment_operator>"][ { "|=" }] = doNothing;

    nodeCreatorRegistry[IOStatement::ID][ { "output", Expression::ID, ";" }] = inputOutputStatement;
    nodeCreatorRegistry[IOStatement::ID][ { "input", Expression::ID, ";" }] = inputOutputStatement;

    nodeCreatorRegistry["<exp_stat>"][ { Expression::ID, ";" }] = expressionStatement;
    nodeCreatorRegistry["<exp_stat>"][ { ";" }] = emptyStatement;

    nodeCreatorRegistry[MATCHED][ { "if", "(", Expression::ID, ")", MATCHED, "else", MATCHED }] = ifElseStatement;
    //nodeCreatorRegistry[MATCHED][ { "switch", "(", Expression::ID, ")", "<matched>" }] = switchStatement;
    //nodeCreatorRegistry[MATCHED][ { "<labeled_stat_matched>" }] = labeledStatement;
    nodeCreatorRegistry[MATCHED][ { "<exp_stat>" }] = doNothing;
    nodeCreatorRegistry[MATCHED][ { Block::ID }] = doNothing;
    nodeCreatorRegistry[MATCHED][ { JumpStatement::ID }] = doNothing;
    nodeCreatorRegistry[MATCHED][ { IOStatement::ID }] = doNothing;

    nodeCreatorRegistry[STATEMENT][ { MATCHED }] = doNothing;
    nodeCreatorRegistry[STATEMENT][ { UNMATCHED }] = doNothing;

    nodeCreatorRegistry[STATEMENTS][ { STATEMENT }] = statementList;
    nodeCreatorRegistry[STATEMENTS][ { STATEMENTS, STATEMENT }] = addToStatementList;

    //nodeCreatorRegistry[JumpStatement::ID][ { "goto", "id" ,";" }] = gotoStatement;
    //nodeCreatorRegistry[JumpStatement::ID][ { "continue", ";" }] = continueStatement;
    //nodeCreatorRegistry[JumpStatement::ID][ { "break", ";" }] = breakStatement;
    nodeCreatorRegistry[JumpStatement::ID][ { "return", Expression::ID, ";" }] = returnExpressionStatement;
    nodeCreatorRegistry[JumpStatement::ID][ { "return", ";" }] = returnVoidStatement;

    nodeCreatorRegistry[ACTUAL_ARGUMENTS][ { AssignmentExpression::ID }] = createActualArgumentsList;
    nodeCreatorRegistry[ACTUAL_ARGUMENTS][ { ACTUAL_ARGUMENTS, ",", AssignmentExpression::ID }] = addToActualArgumentsList;

    nodeCreatorRegistry[Block::ID][ { "{", DECLARATIONS, STATEMENTS, "}" }] = fullCompound;
    nodeCreatorRegistry[Block::ID][ { "{", STATEMENTS, "}" }] = statementCompound;
    nodeCreatorRegistry[Block::ID][ { "{", DECLARATIONS, "}" }] = declarationCompound;
    nodeCreatorRegistry[Block::ID][ { "{", "}" }] = emptyCompound;

    //nodeCreatorRegistry[FunctionDefinition::ID][ { DeclarationSpecifiers::ID, Declarator::ID, DECLARATIONS, Block::ID }] = deprecatedFunctionDefinition;
    //nodeCreatorRegistry[FunctionDefinition::ID][ { Declarator::ID, DECLARATIONS, Block::ID }] = deprecatedDefaultReturnFunctionDefinition;
    nodeCreatorRegistry[FunctionDefinition::ID][ { DeclarationSpecifiers::ID, Declarator::ID, Block::ID }] = functionDefinition;
    nodeCreatorRegistry[FunctionDefinition::ID][ { Declarator::ID, Block::ID }] = defaultReturnTypeFunctionDefinition;

    nodeCreatorRegistry[EXTERNAL_DECLARATION][ { FunctionDefinition::ID }] = externalFunctionDefinition;
    nodeCreatorRegistry[EXTERNAL_DECLARATION][ { Declaration::ID }] = externalDeclaration;

    nodeCreatorRegistry[TRANSLATION_UNIT][ { EXTERNAL_DECLARATION }] = translationUnit;
    nodeCreatorRegistry[TRANSLATION_UNIT][ { TRANSLATION_UNIT, EXTERNAL_DECLARATION }] = addToTranslationUnit;

    nodeCreatorRegistry[MATCHED][ { ITERATION_STATEMENT_MATCHED }] = doNothing;
    nodeCreatorRegistry[UNMATCHED][ { ITERATION_STATEMENT_UNMATCHED }] = doNothing;

    nodeCreatorRegistry[ITERATION_STATEMENT_MATCHED][ { "while", "(", Expression::ID, ")", MATCHED }] = whileLoopStatement;
    nodeCreatorRegistry[ITERATION_STATEMENT_UNMATCHED][ { "while", "(", Expression::ID, ")", UNMATCHED }] = whileLoopStatement;
    nodeCreatorRegistry[ITERATION_STATEMENT_MATCHED][ { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")", MATCHED }] = forLoopStatement;
    nodeCreatorRegistry[ITERATION_STATEMENT_UNMATCHED][ { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")", UNMATCHED }] = forLoopStatement;


    /*
     nodeCreatorRegistry[UNMATCHED][ { "if", "(", Expression::ID, ")", STATEMENT }] = ContextualSyntaxNodeBuilder::ifStatement;
     nodeCreatorRegistry[UNMATCHED][ { "if", "(", Expression::ID, ")", MATCHED, "else", UNMATCHED }] = ContextualSyntaxNodeBuilder::ifElseStatement;
     nodeCreatorRegistry[UNMATCHED][ { LoopHeader::ID, UNMATCHED }] = ContextualSyntaxNodeBuilder::loopStatement;

     nodeCreatorRegistry[VariableDeclaration::ID][ { TYPE_SPECIFIER, DeclarationList::ID, ";" }] = ContextualSyntaxNodeBuilder::variableDeclaration;
     nodeCreatorRegistry[VariableDeclaration::ID][ { TYPE_SPECIFIER, DeclarationList::ID, "=", AssignmentExpression::ID, ";" }] =
     ContextualSyntaxNodeBuilder::variableDefinition;

     nodeCreatorRegistry[VAR_DECLARATIONS][ { VAR_DECLARATION }] = ContextualSyntaxNodeBuilder::newListCarrier;
     nodeCreatorRegistry[VAR_DECLARATIONS][ { VAR_DECLARATIONS, VAR_DECLARATION }] = ContextualSyntaxNodeBuilder::addToListCarrier;

     nodeCreatorRegistry[FUNCTION_DECLARATIONS][ { FunctionDefinition::ID }] = ContextualSyntaxNodeBuilder::newListCarrier;
     nodeCreatorRegistry[FUNCTION_DECLARATIONS][ { FUNCTION_DECLARATIONS, FunctionDefinition::ID }] = ContextualSyntaxNodeBuilder::addToListCarrier;

     nodeCreatorRegistry[TRANSLATION_UNIT][ { FUNCTION_DECLARATIONS }] = ContextualSyntaxNodeBuilder::functionsTranslationUnit;
     nodeCreatorRegistry[TRANSLATION_UNIT][ { VAR_DECLARATIONS, FUNCTION_DECLARATIONS }] =
     ContextualSyntaxNodeBuilder::variablesFunctionsTranslationUnit;
     */

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

void ContextualSyntaxNodeBuilder::noCreatorDefined(std::string definingSymbol, const std::vector<std::string>& production) const {
    std::ostringstream productionString;
    for (auto& symbol : production) {
        productionString << symbol << " ";
    }
    throw std::runtime_error { "no AST creator defined for production `" + definingSymbol + " ::= " + productionString.str() + "`" };
}

void ContextualSyntaxNodeBuilder::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::make_unique<JumpStatement>(context.popTerminal()));
    context.popTerminal();
}

void ContextualSyntaxNodeBuilder::whileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto loopHeader = std::make_unique<WhileLoopHeader>(context.popExpression());
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void ContextualSyntaxNodeBuilder::forLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto increment = context.popExpression();
    auto clause = context.popExpression();
    auto initialization = context.popExpression();
    auto loopHeader = std::make_unique<ForLoopHeader>(std::move(initialization), std::move(clause), std::move(increment));
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void ContextualSyntaxNodeBuilder::ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<IfStatement>(context.popExpression(), context.popStatement()));
}

void ContextualSyntaxNodeBuilder::loopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::make_unique<LoopStatement>(context.popLoopHeader(), context.popStatement()));
}

} /* namespace ast */
