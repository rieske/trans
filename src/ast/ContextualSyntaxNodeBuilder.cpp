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

#include "ast/StringLiteralExpression.h"
#include "types/Type.h"

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
    context.pushTypeSpecifier( { type::signedInteger(), context.popTerminal().value });
}

void longType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "long type is not implemented yet" };
}

void characterType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedCharacter(), context.popTerminal().value });
}

void voidType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::voidType(), context.popTerminal().value });
}

void floatType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::floating(), context.popTerminal().value });
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
    context.pushTypeQualifier(type::Qualifier::CONST);
}

void volatileQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeQualifier(type::Qualifier::VOLATILE);
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
    context.pushConstant( { constant.value, type::signedInteger(), constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedCharacter(), constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    throw std::runtime_error { "floating constants not implemented yet" };
    // context.pushConstant( { constant.value, type::floating(), constant.context });
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

void stringLiteralExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto literal = context.popTerminal();
    context.pushExpression(std::make_unique<StringLiteralExpression>(literal.value, literal.context));
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

void ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<IfStatement>(context.popExpression(), context.popStatement()));
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

void forLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
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

void whileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto loopHeader = std::make_unique<WhileLoopHeader>(context.popExpression());
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
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
    DeclarationSpecifiers defaultReturnTypeSpecifiers { TypeSpecifier { type::signedInteger(), "int" } };
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

ContextualSyntaxNodeBuilder::ContextualSyntaxNodeBuilder(const parser::Grammar& grammar) {
    this->grammar = &grammar;

    int s_typeSpecifier = grammar.symbolId(TYPE_SPECIFIER);
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("short") }] = shortType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("int") }] = integerType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("long") }] = longType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("char") }] = characterType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("void") }] = voidType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("float") }] = floatType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("double") }] = doubleType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("signed") }] = signedType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("unsigned") }] = unsignedType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("typedef_name") }] = typedefName;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("<struct_or_union_spec>") }] = structOrUnionType;
    nodeCreatorRegistry[s_typeSpecifier][{ grammar.symbolId("<enum_spec>") }] = enumType;

    int s_typeQualifier = grammar.symbolId(TYPE_QUALIFIER);
    nodeCreatorRegistry[s_typeQualifier][{ grammar.symbolId("const") }] = constQualifier;
    nodeCreatorRegistry[s_typeQualifier][{ grammar.symbolId("volatile") }] = volatileQualifier;

    int s_declarationSpecifiers = grammar.symbolId(DeclarationSpecifiers::ID);
    nodeCreatorRegistry[s_declarationSpecifiers][{ s_typeSpecifier }] = declarationTypeSpecifier;
    nodeCreatorRegistry[s_declarationSpecifiers][{ s_typeSpecifier, s_declarationSpecifiers }] = addDeclarationTypeSpecifier;
    nodeCreatorRegistry[s_declarationSpecifiers][{ grammar.symbolId("<storage_class_spec>") }] = declarationStorageClassSpecifier;
    nodeCreatorRegistry[s_declarationSpecifiers][{ grammar.symbolId("<storage_class_spec>"), s_declarationSpecifiers }] = addDeclarationStorageClassSpecifier;
    nodeCreatorRegistry[s_declarationSpecifiers][{ s_typeQualifier }] = declarationTypeQualifier;
    nodeCreatorRegistry[s_declarationSpecifiers][{ s_typeQualifier, s_declarationSpecifiers }] = addDeclarationTypeQualifier;

    int s_directDeclarator = grammar.symbolId(DirectDeclarator::ID);
    int s_declarator = grammar.symbolId(Declarator::ID);
    int s_formalArgumentsDecl = grammar.symbolId(FORMAL_ARGUMENTS_DECLARATION);
    nodeCreatorRegistry[s_directDeclarator][{ grammar.symbolId("id") }] = identifierDeclarator;
    nodeCreatorRegistry[s_directDeclarator][{ grammar.symbolId("("), s_declarator, grammar.symbolId(")") }] = parenthesizedExpression;
    nodeCreatorRegistry[s_directDeclarator][{ s_directDeclarator, grammar.symbolId("["), grammar.symbolId("<const_exp>"), grammar.symbolId("]") }] = arrayDeclarator;
    nodeCreatorRegistry[s_directDeclarator][{ s_directDeclarator, grammar.symbolId("["), grammar.symbolId("]") }] = abstractArrayDeclarator;
    nodeCreatorRegistry[s_directDeclarator][{ s_directDeclarator, grammar.symbolId("("), s_formalArgumentsDecl, grammar.symbolId(")") }] = functionDeclarator;
    nodeCreatorRegistry[s_directDeclarator][{ s_directDeclarator, grammar.symbolId("("), grammar.symbolId(")") }] = noargFunctionDeclarator;

    int s_pointer = grammar.symbolId(Pointer::ID);
    nodeCreatorRegistry[s_declarator][{ s_pointer, s_directDeclarator }] = pointerToDeclarator;
    nodeCreatorRegistry[s_declarator][{ s_directDeclarator }] = declarator;

    int s_formalArg = grammar.symbolId(FormalArgument::ID);
    nodeCreatorRegistry[s_formalArg][{ s_declarationSpecifiers, s_declarator }] = parameterDeclaration;
    nodeCreatorRegistry[s_formalArg][{ s_declarationSpecifiers, grammar.symbolId("<abstract_declarator>") }] = abstractParameterDeclaration;
    nodeCreatorRegistry[s_formalArg][{ s_declarationSpecifiers }] = parameterBaseTypeDeclaration;

    int s_formalArgs = grammar.symbolId(FORMAL_ARGUMENTS);
    nodeCreatorRegistry[s_formalArgs][{ s_formalArg }] = formalArguments;
    nodeCreatorRegistry[s_formalArgs][{ s_formalArgs, grammar.symbolId(","), s_formalArg }] = addFormalArgument;

    nodeCreatorRegistry[s_formalArgumentsDecl][{ s_formalArgs }] = formalArgumentsDeclaration;
    nodeCreatorRegistry[s_formalArgumentsDecl][{ s_formalArgs, grammar.symbolId(","), grammar.symbolId("...") }] = formalArgumentsWithVararg;

    int s_constant = grammar.symbolId(CONSTANT);
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("int_const") }] = integerConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("char_const") }] = characterConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("float_const") }] = floatConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("enumeration_const") }] = enumerationConstant;

    int s_expression = grammar.symbolId(Expression::ID);
    int s_primaryExpression = grammar.symbolId(PRIMARY_EXPRESSION);
    nodeCreatorRegistry[s_primaryExpression][{ grammar.symbolId("id") }] = identifierExpression;
    nodeCreatorRegistry[s_primaryExpression][{ s_constant }] = constantExpression;
    nodeCreatorRegistry[s_primaryExpression][{ grammar.symbolId("string") }] = stringLiteralExpression;
    nodeCreatorRegistry[s_primaryExpression][{ grammar.symbolId("("), s_expression, grammar.symbolId(")") }] = parenthesizedExpression;

    int s_actualArgs = grammar.symbolId(ACTUAL_ARGUMENTS);
    int s_postfixExpression = grammar.symbolId(PostfixExpression::ID);
    nodeCreatorRegistry[s_postfixExpression][{ s_primaryExpression }] = doNothing;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId("["), s_expression, grammar.symbolId("]") }] = arrayAccess;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId("("), s_actualArgs, grammar.symbolId(")") }] = functionCall;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId("("), grammar.symbolId(")") }] = noargFunctionCall;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId(".") }] = directMemberAccess;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId("->") }] = pointeeMemberAccess;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId("++") }] = postfixIncrementDecrement;
    nodeCreatorRegistry[s_postfixExpression][{ s_postfixExpression, grammar.symbolId("--") }] = postfixIncrementDecrement;

    int s_typeCast = grammar.symbolId(TypeCast::ID);
    int s_unaryExpression = grammar.symbolId(UnaryExpression::ID);
    nodeCreatorRegistry[s_unaryExpression][{ s_postfixExpression }] = doNothing;
    nodeCreatorRegistry[s_unaryExpression][{ grammar.symbolId("++"), s_unaryExpression }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unaryExpression][{ grammar.symbolId("--"), s_unaryExpression }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unaryExpression][{ grammar.symbolId("<unary_operator>"), s_typeCast }] = unaryExpression;
    nodeCreatorRegistry[s_unaryExpression][{ grammar.symbolId("sizeof"), s_unaryExpression }] = sizeofExpression;
    nodeCreatorRegistry[s_unaryExpression][{ grammar.symbolId("sizeof"), grammar.symbolId("("), grammar.symbolId("<type_name>"), grammar.symbolId(")") }] = sizeofTypeExpression;

    nodeCreatorRegistry[s_typeCast][{ s_unaryExpression }] = doNothing;
    nodeCreatorRegistry[s_typeCast][{ grammar.symbolId("("), grammar.symbolId("<type_name>"), grammar.symbolId(")"), s_typeCast }] = typeCast;

    int s_multiplication = grammar.symbolId(ArithmeticExpression::MULTIPLICATION);
    nodeCreatorRegistry[s_multiplication][{ s_typeCast }] = doNothing;
    nodeCreatorRegistry[s_multiplication][{ s_multiplication, grammar.symbolId("*"), s_typeCast }] = arithmeticExpression;
    nodeCreatorRegistry[s_multiplication][{ s_multiplication, grammar.symbolId("/"), s_typeCast }] = arithmeticExpression;
    nodeCreatorRegistry[s_multiplication][{ s_multiplication, grammar.symbolId("%"), s_typeCast }] = arithmeticExpression;

    int s_addition = grammar.symbolId(ArithmeticExpression::ADDITION);
    nodeCreatorRegistry[s_addition][{ s_multiplication }] = doNothing;
    nodeCreatorRegistry[s_addition][{ s_addition, grammar.symbolId("+"), s_multiplication }] = arithmeticExpression;
    nodeCreatorRegistry[s_addition][{ s_addition, grammar.symbolId("-"), s_multiplication }] = arithmeticExpression;

    int s_shift = grammar.symbolId(ShiftExpression::ID);
    nodeCreatorRegistry[s_shift][{ s_addition }] = doNothing;
    nodeCreatorRegistry[s_shift][{ s_shift, grammar.symbolId("<<"), s_addition }] = shiftExpression;
    nodeCreatorRegistry[s_shift][{ s_shift, grammar.symbolId(">>"), s_addition }] = shiftExpression;

    int s_relationalComparison = grammar.symbolId(ComparisonExpression::RELATIONAL);
    nodeCreatorRegistry[s_relationalComparison][{ s_shift }] = doNothing;
    nodeCreatorRegistry[s_relationalComparison][{ s_relationalComparison, grammar.symbolId("<"), s_shift }] = relationalExpression;
    nodeCreatorRegistry[s_relationalComparison][{ s_relationalComparison, grammar.symbolId(">"), s_shift }] = relationalExpression;
    nodeCreatorRegistry[s_relationalComparison][{ s_relationalComparison, grammar.symbolId("<="), s_shift }] = relationalExpression;
    nodeCreatorRegistry[s_relationalComparison][{ s_relationalComparison, grammar.symbolId(">="), s_shift }] = relationalExpression;

    int s_equalityComparison = grammar.symbolId(ComparisonExpression::EQUALITY);
    nodeCreatorRegistry[s_equalityComparison][{ s_relationalComparison }] = doNothing;
    nodeCreatorRegistry[s_equalityComparison][{ s_equalityComparison, grammar.symbolId("=="), s_relationalComparison }] = relationalExpression;
    nodeCreatorRegistry[s_equalityComparison][{ s_equalityComparison, grammar.symbolId("!="), s_relationalComparison }] = relationalExpression;

    int s_bitwiseAnd = grammar.symbolId(BitwiseExpression::AND);
    nodeCreatorRegistry[s_bitwiseAnd][{ s_equalityComparison }] = doNothing;
    nodeCreatorRegistry[s_bitwiseAnd][{ s_bitwiseAnd, grammar.symbolId("&"), s_equalityComparison }] = bitwiseExpression;

    int s_bitwiseXor = grammar.symbolId(BitwiseExpression::XOR);
    nodeCreatorRegistry[s_bitwiseXor][{ s_bitwiseAnd }] = doNothing;
    nodeCreatorRegistry[s_bitwiseXor][{ s_bitwiseXor, grammar.symbolId("^"), s_bitwiseAnd }] = bitwiseExpression;

    int s_bitwiseOr = grammar.symbolId(BitwiseExpression::OR);
    nodeCreatorRegistry[s_bitwiseOr][{ s_bitwiseXor }] = doNothing;
    nodeCreatorRegistry[s_bitwiseOr][{ s_bitwiseOr, grammar.symbolId("|"), s_bitwiseXor }] = bitwiseExpression;

    int s_logicalAnd = grammar.symbolId(LogicalAndExpression::ID);
    nodeCreatorRegistry[s_logicalAnd][{ s_bitwiseOr }] = doNothing;
    nodeCreatorRegistry[s_logicalAnd][{ s_logicalAnd, grammar.symbolId("&&"), s_bitwiseOr }] = logicalAndExpression;

    int s_logicalOr = grammar.symbolId(LogicalOrExpression::ID);
    nodeCreatorRegistry[s_logicalOr][{ s_logicalAnd }] = doNothing;
    nodeCreatorRegistry[s_logicalOr][{ s_logicalOr, grammar.symbolId("||"), s_logicalAnd }] = logicalOrExpression;

    int s_conditional = grammar.symbolId("<conditional_exp>");
    nodeCreatorRegistry[s_conditional][{ s_logicalOr }] = doNothing;
    nodeCreatorRegistry[s_conditional][{ s_logicalOr, grammar.symbolId("?"), grammar.symbolId("<exp>"), grammar.symbolId(":"), s_conditional }] = conditionalExpression;

    int s_assignment = grammar.symbolId(AssignmentExpression::ID);
    nodeCreatorRegistry[s_assignment][{ s_conditional }] = doNothing;
    nodeCreatorRegistry[s_assignment][{ s_unaryExpression, grammar.symbolId("<assignment_operator>"), s_assignment }] = assignmentExpression;

    int s_initializer = grammar.symbolId("<initializer>");
    nodeCreatorRegistry[s_initializer][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_initializer][{ grammar.symbolId("{"), grammar.symbolId("<initializer_list>"), grammar.symbolId("}") }] = initializer;

    int s_initializedDeclarator = grammar.symbolId(InitializedDeclarator::ID);
    nodeCreatorRegistry[s_initializedDeclarator][{ s_declarator }] = initializedDeclarator;
    nodeCreatorRegistry[s_initializedDeclarator][{ s_declarator, grammar.symbolId("="), s_initializer }] = initializedDeclaratorWithInitializer;

    int s_initDeclaratorList = grammar.symbolId("<init_declarator_list>");
    nodeCreatorRegistry[s_initDeclaratorList][{ s_initializedDeclarator }] = initializedDeclaratorList;
    nodeCreatorRegistry[s_initDeclaratorList][{s_initDeclaratorList, grammar.symbolId(","), s_initializedDeclarator }] = addToInitializedDeclaratorList;

    int s_declaration = grammar.symbolId(Declaration::ID);
    nodeCreatorRegistry[s_declaration][{ s_declarationSpecifiers, grammar.symbolId("<init_declarator_list>"), grammar.symbolId(";") }] = initializedDeclaration;
    nodeCreatorRegistry[s_declaration][{ s_declarationSpecifiers, grammar.symbolId(";") }] = declaration;

    int s_declarations = grammar.symbolId(DECLARATIONS);
    nodeCreatorRegistry[s_declarations][{ s_declaration }] = declarationList;
    nodeCreatorRegistry[s_declarations][{ s_declarations, s_declaration }] = addDeclarationToList;

    nodeCreatorRegistry[s_expression][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_expression][{ s_expression, grammar.symbolId(","), s_assignment }] = expressionList;

    int s_typeQualifierList = grammar.symbolId(TYPE_QUALIFIER_LIST);
    nodeCreatorRegistry[s_typeQualifierList][{ s_typeQualifier }] = typeQualifierList;
    nodeCreatorRegistry[s_typeQualifierList][{ s_typeQualifierList, s_typeQualifier }] = addTypeQualifierToList;

    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_typeQualifierList }] = qualifiedPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*") }] = pointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_typeQualifierList, s_pointer }] = qualifiedPointerToPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_pointer }] = pointerToPointer;

    int s_unaryOperator = grammar.symbolId("<unary_operator>");
    nodeCreatorRegistry[s_unaryOperator][{ grammar.symbolId("&") }] = doNothing;
    nodeCreatorRegistry[s_unaryOperator][{ grammar.symbolId("*") }] = doNothing;
    nodeCreatorRegistry[s_unaryOperator][{ grammar.symbolId("+") }] = doNothing;
    nodeCreatorRegistry[s_unaryOperator][{ grammar.symbolId("-") }] = doNothing;
    nodeCreatorRegistry[s_unaryOperator][{ grammar.symbolId("~") }] = doNothing;
    nodeCreatorRegistry[s_unaryOperator][{ grammar.symbolId("!") }] = doNothing;

    int s_assignmentOperator = grammar.symbolId("<assignment_operator>");
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("*=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("/=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("%=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("+=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("-=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("<<=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId(">>=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("&=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("^=") }] = doNothing;
    nodeCreatorRegistry[s_assignmentOperator][{ grammar.symbolId("|=") }] = doNothing;

    int s_expressionStatement = grammar.symbolId("<exp_stat>");
    nodeCreatorRegistry[s_expressionStatement][{ s_expression, grammar.symbolId(";") }] = expressionStatement;
    nodeCreatorRegistry[s_expressionStatement][{ grammar.symbolId(";") }] = emptyStatement;

    int s_matched = grammar.symbolId(MATCHED);
    int s_unmatched = grammar.symbolId(UNMATCHED);
    int s_stmt = grammar.symbolId(STATEMENT);
    int s_block = grammar.symbolId(Block::ID);
    int s_jump = grammar.symbolId(JumpStatement::ID);
    int s_if = grammar.symbolId("if");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    nodeCreatorRegistry[s_matched][{s_if, s_open_paren, s_expression, s_close_paren, s_matched, grammar.symbolId("else"), s_matched }] = ifElseStatement;
    nodeCreatorRegistry[s_unmatched][{s_if, s_open_paren, s_expression, s_close_paren, s_stmt }] = ifStatement;
    //nodeCreatorRegistry[s_matched][{ grammar.symbolId("switch"), s_open_paren, s_expression, s_close_paren, s_matched }] = switchStatement;
    //nodeCreatorRegistry[s_matched][{ grammar.symbolId("<labeled_stat_matched>") }] = labeledStatement; // ??
    nodeCreatorRegistry[s_matched][{ s_expressionStatement }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_block }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_jump }] = doNothing;

    nodeCreatorRegistry[s_stmt][{ s_matched }] = doNothing;
    nodeCreatorRegistry[s_stmt][{ s_unmatched }] = doNothing;

    int s_stmts = grammar.symbolId(STATEMENTS);
    nodeCreatorRegistry[s_stmts][{ s_stmt }] = statementList;
    nodeCreatorRegistry[s_stmts][{ s_stmts, s_stmt }] = addToStatementList;

    int s_return = grammar.symbolId("return");
    int s_semicolon = grammar.symbolId(";");
    //nodeCreatorRegistry[s_jump][{ grammar.symbolId("goto"), grammar.symbolId("id"), s_semicolon }] = gotoStatement;
    //nodeCreatorRegistry[s_jump][{ grammar.symbolId("continue"), s_semicolon }] = continueStatement;
    //nodeCreatorRegistry[s_jump][{ grammar.symbolId("break"), s_semicolon }] = breakStatement;
    nodeCreatorRegistry[s_jump][{ s_return, s_expression, s_semicolon }] = returnExpressionStatement;
    nodeCreatorRegistry[s_jump][{ s_return, s_semicolon }] = returnVoidStatement;

    nodeCreatorRegistry[s_actualArgs][{ s_assignment }] = createActualArgumentsList;
    nodeCreatorRegistry[s_actualArgs][{ s_actualArgs, grammar.symbolId(","), s_assignment }] = addToActualArgumentsList;

    int s_open_brace = grammar.symbolId("{");
    int s_close_brace = grammar.symbolId("}");
    nodeCreatorRegistry[s_block][{ s_open_brace, s_declarations, s_stmts, s_close_brace}] = fullCompound;
    nodeCreatorRegistry[s_block][{ s_open_brace, s_stmts, s_close_brace}] = statementCompound;
    nodeCreatorRegistry[s_block][{ s_open_brace, s_declarations, s_close_brace}] = declarationCompound;
    nodeCreatorRegistry[s_block][{ s_open_brace, s_close_brace}] = emptyCompound;

    int s_functionDefinition = grammar.symbolId(FunctionDefinition::ID);
    nodeCreatorRegistry[s_functionDefinition][{ s_declarationSpecifiers, s_declarator, s_block }] = functionDefinition;
    nodeCreatorRegistry[s_functionDefinition][{ s_declarator, s_block }] = defaultReturnTypeFunctionDefinition;

    int s_externalDeclaration = grammar.symbolId(EXTERNAL_DECLARATION);
    nodeCreatorRegistry[s_externalDeclaration][{ s_functionDefinition }] = externalFunctionDefinition;
    nodeCreatorRegistry[s_externalDeclaration][{ s_declaration }] = externalDeclaration;

    int s_translationUnit = grammar.symbolId(TRANSLATION_UNIT);
    nodeCreatorRegistry[s_translationUnit][{ s_externalDeclaration }] = translationUnit;
    nodeCreatorRegistry[s_translationUnit][{ s_translationUnit, s_externalDeclaration }] = addToTranslationUnit;

    int s_iterationMatched = grammar.symbolId(ITERATION_STATEMENT_MATCHED);
    int s_iterationUnmatched = grammar.symbolId(ITERATION_STATEMENT_UNMATCHED);
    nodeCreatorRegistry[s_matched][{ s_iterationMatched }] = doNothing;
    nodeCreatorRegistry[s_unmatched][{ s_iterationUnmatched }] = doNothing;

    int s_while = grammar.symbolId("while");
    int s_for = grammar.symbolId("for");
    nodeCreatorRegistry[s_iterationMatched][{ s_while, s_open_paren, s_expression, s_close_paren, s_matched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iterationUnmatched][{ s_while, s_open_paren, s_expression, s_close_paren, s_unmatched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iterationMatched][{ s_for, s_open_paren, s_expression, s_semicolon, s_expression, s_semicolon, s_expression, s_close_paren, s_matched }] = forLoopStatement;
    nodeCreatorRegistry[s_iterationUnmatched][{ s_for, s_open_paren, s_expression, s_semicolon, s_expression, s_semicolon, s_expression, s_close_paren, s_unmatched }] = forLoopStatement;
}

ContextualSyntaxNodeBuilder::~ContextualSyntaxNodeBuilder() = default;

void ContextualSyntaxNodeBuilder::updateContext(std::string definingSymbol, const std::vector<int>& production,
        AbstractSyntaxTreeBuilderContext& context) const
{
    try {
        nodeCreatorRegistry.at(std::stoi(definingSymbol)).at(production)(context);
    } catch (std::out_of_range& exception) {
        noCreatorDefined(definingSymbol, production);
    }
}

void ContextualSyntaxNodeBuilder::noCreatorDefined(std::string definingSymbol, const std::vector<int>& production) const {
    // TODO: lookup symbols from production symbol ids to report the error in an understandable way
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

} /* namespace ast */
