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

    int s_type_specifier = grammar.symbolId("<type_spec>");
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("short") }] = shortType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("int") }] = integerType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("long") }] = longType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("char") }] = characterType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("void") }] = voidType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("float") }] = floatType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("double") }] = doubleType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("signed") }] = signedType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("unsigned") }] = unsignedType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("typedef_name") }] = typedefName;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<struct_or_union_spec>") }] = structOrUnionType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<enum_spec>") }] = enumType;

    int s_type_qualifier = grammar.symbolId("<type_qualifier>");
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("const") }] = constQualifier;
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("volatile") }] = volatileQualifier;

    int s_decl_specs = grammar.symbolId("<decl_specs>");
    nodeCreatorRegistry[s_decl_specs][{ s_type_specifier }] = declarationTypeSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_specifier, s_decl_specs }] = addDeclarationTypeSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ grammar.symbolId("<storage_class_spec>") }] = declarationStorageClassSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ grammar.symbolId("<storage_class_spec>"), s_decl_specs }] = addDeclarationStorageClassSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_qualifier }] = declarationTypeQualifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_qualifier, s_decl_specs }] = addDeclarationTypeQualifier;

    int s_direct_declarator = grammar.symbolId("<direct_declarator>");
    int s_declarator = grammar.symbolId("<declarator>");
    int s_param_type_list = grammar.symbolId("<param_type_list>");
    int s_identifier = grammar.symbolId("id");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_open_bracket = grammar.symbolId("[");
    int s_close_bracket = grammar.symbolId("]");

    nodeCreatorRegistry[s_direct_declarator][{ s_identifier }] = identifierDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_open_paren, s_declarator, s_close_paren }] = parenthesizedExpression;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] = arrayDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, s_close_bracket }] = abstractArrayDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_param_type_list, s_close_paren }] = functionDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_close_paren }] = noargFunctionDeclarator;

    int s_pointer = grammar.symbolId("<pointer>" );
    nodeCreatorRegistry[s_declarator][{ s_pointer, s_direct_declarator }] = pointerToDeclarator;
    nodeCreatorRegistry[s_declarator][{ s_direct_declarator }] = declarator;

    int s_param_decl = grammar.symbolId("<param_decl>");
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs, s_declarator }] = parameterDeclaration;
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs, grammar.symbolId("<abstract_declarator>") }] = abstractParameterDeclaration;
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs }] = parameterBaseTypeDeclaration;

    int s_param_list = grammar.symbolId("<param_list>");
    int s_comma = grammar.symbolId(",");
    nodeCreatorRegistry[s_param_list][{ s_param_decl }] = formalArguments;
    nodeCreatorRegistry[s_param_list][{ s_param_list, s_comma, s_param_decl }] = addFormalArgument;

    nodeCreatorRegistry[s_param_type_list][{ s_param_list }] = formalArgumentsDeclaration;
    nodeCreatorRegistry[s_param_type_list][{ s_param_list, s_comma, grammar.symbolId("...") }] = formalArgumentsWithVararg;

    int s_constant = grammar.symbolId("<const>");
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("int_const") }] = integerConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("char_const") }] = characterConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("float_const") }] = floatConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("enumeration_const") }] = enumerationConstant;

    int s_exp = grammar.symbolId("<exp>");
    int s_primary_exp = grammar.symbolId("<primary_exp>");
    nodeCreatorRegistry[s_primary_exp][{ s_identifier }] = identifierExpression;
    nodeCreatorRegistry[s_primary_exp][{ s_constant }] = constantExpression;
    nodeCreatorRegistry[s_primary_exp][{ grammar.symbolId("string") }] = stringLiteralExpression;
    nodeCreatorRegistry[s_primary_exp][{ s_open_paren, s_exp, s_close_paren }] = parenthesizedExpression;

    int s_argument_exp_list = grammar.symbolId("<argument_exp_list>");
    int s_postfix_exp = grammar.symbolId("<postfix_exp>");
    nodeCreatorRegistry[s_postfix_exp][{ s_primary_exp }] = doNothing;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, s_open_bracket, s_exp, s_close_bracket }] = arrayAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, s_open_paren, s_argument_exp_list, s_close_paren }] = functionCall;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, s_open_paren, s_close_paren }] = noargFunctionCall;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId(".") }] = directMemberAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("->") }] = pointeeMemberAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("++") }] = postfixIncrementDecrement;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("--") }] = postfixIncrementDecrement;

    int s_cast_exp = grammar.symbolId("<cast_exp>");
    int s_unary_exp = grammar.symbolId("<unary_exp>");
    int s_unary_operator = grammar.symbolId("<unary_operator>");
    nodeCreatorRegistry[s_unary_exp][{ s_postfix_exp }] = doNothing;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("++"), s_unary_exp }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("--"), s_unary_exp }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unary_exp][{ s_unary_operator, s_cast_exp }] = unaryExpression;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("sizeof"), s_unary_exp }] = sizeofExpression;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("sizeof"), s_open_paren, grammar.symbolId("<type_name>"), s_close_paren }] = sizeofTypeExpression;

    nodeCreatorRegistry[s_cast_exp][{ s_unary_exp }] = doNothing;
    nodeCreatorRegistry[s_cast_exp][{ s_open_paren, grammar.symbolId("<type_name>"), s_close_paren, s_cast_exp }] = typeCast;

    int s_mult_exp = grammar.symbolId("<mult_exp>");
    nodeCreatorRegistry[s_mult_exp][{ s_cast_exp }] = doNothing;
    nodeCreatorRegistry[s_mult_exp][{ s_mult_exp, grammar.symbolId("*"), s_cast_exp }] = arithmeticExpression;
    nodeCreatorRegistry[s_mult_exp][{ s_mult_exp, grammar.symbolId("/"), s_cast_exp }] = arithmeticExpression;
    nodeCreatorRegistry[s_mult_exp][{ s_mult_exp, grammar.symbolId("%"), s_cast_exp }] = arithmeticExpression;

    int s_additive_exp = grammar.symbolId("<additive_exp>");
    nodeCreatorRegistry[s_additive_exp][{ s_mult_exp }] = doNothing;
    nodeCreatorRegistry[s_additive_exp][{ s_additive_exp, grammar.symbolId("+"), s_mult_exp }] = arithmeticExpression;
    nodeCreatorRegistry[s_additive_exp][{ s_additive_exp, grammar.symbolId("-"), s_mult_exp }] = arithmeticExpression;

    int s_shift_exp = grammar.symbolId("<shift_expression>");
    nodeCreatorRegistry[s_shift_exp][{ s_additive_exp }] = doNothing;
    nodeCreatorRegistry[s_shift_exp][{ s_shift_exp, grammar.symbolId("<<"), s_additive_exp }] = shiftExpression;
    nodeCreatorRegistry[s_shift_exp][{ s_shift_exp, grammar.symbolId(">>"), s_additive_exp }] = shiftExpression;

    int s_relational_exp = grammar.symbolId("<relational_exp>");
    nodeCreatorRegistry[s_relational_exp][{ s_shift_exp }] = doNothing;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId("<"), s_shift_exp }] = relationalExpression;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId(">"), s_shift_exp }] = relationalExpression;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId("<="), s_shift_exp }] = relationalExpression;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId(">="), s_shift_exp }] = relationalExpression;

    int s_equality_exp = grammar.symbolId("<equality_exp>");
    nodeCreatorRegistry[s_equality_exp][{ s_relational_exp }] = doNothing;
    nodeCreatorRegistry[s_equality_exp][{ s_equality_exp, grammar.symbolId("=="), s_relational_exp }] = relationalExpression;
    nodeCreatorRegistry[s_equality_exp][{ s_equality_exp, grammar.symbolId("!="), s_relational_exp }] = relationalExpression;

    int s_and_exp = grammar.symbolId("<and_exp>");
    nodeCreatorRegistry[s_and_exp][{ s_equality_exp }] = doNothing;
    nodeCreatorRegistry[s_and_exp][{ s_and_exp, grammar.symbolId("&"), s_equality_exp }] = bitwiseExpression;

    int s_exclusive_or_exp = grammar.symbolId("<exclusive_or_exp>");
    nodeCreatorRegistry[s_exclusive_or_exp][{ s_and_exp }] = doNothing;
    nodeCreatorRegistry[s_exclusive_or_exp][{ s_exclusive_or_exp, grammar.symbolId("^"), s_and_exp }] = bitwiseExpression;

    int s_inclusive_or_exp = grammar.symbolId("<inclusive_or_exp>");
    nodeCreatorRegistry[s_inclusive_or_exp][{ s_exclusive_or_exp }] = doNothing;
    nodeCreatorRegistry[s_inclusive_or_exp][{ s_inclusive_or_exp, grammar.symbolId("|"), s_exclusive_or_exp }] = bitwiseExpression;

    int s_logical_and_exp = grammar.symbolId("<logical_and_exp>");
    nodeCreatorRegistry[s_logical_and_exp][{ s_inclusive_or_exp }] = doNothing;
    nodeCreatorRegistry[s_logical_and_exp][{ s_logical_and_exp, grammar.symbolId("&&"), s_inclusive_or_exp }] = logicalAndExpression;

    int s_logical_or_exp = grammar.symbolId("<logical_or_exp>");
    nodeCreatorRegistry[s_logical_or_exp][{ s_logical_and_exp }] = doNothing;
    nodeCreatorRegistry[s_logical_or_exp][{ s_logical_or_exp, grammar.symbolId("||"), s_logical_and_exp }] = logicalOrExpression;

    int s_conditional_exp = grammar.symbolId("<conditional_exp>");
    nodeCreatorRegistry[s_conditional_exp][{ s_logical_or_exp }] = doNothing;
    nodeCreatorRegistry[s_conditional_exp][{ s_logical_or_exp, grammar.symbolId("?"), s_exp, grammar.symbolId(":"), s_conditional_exp }] = conditionalExpression;

    int s_assignment = grammar.symbolId("<assignment_exp>");
    int s_assignment_operator = grammar.symbolId("<assignment_operator>");
    nodeCreatorRegistry[s_assignment][{ s_conditional_exp }] = doNothing;
    nodeCreatorRegistry[s_assignment][{ s_unary_exp, s_assignment_operator, s_assignment }] = assignmentExpression;

    int s_initializer = grammar.symbolId("<initializer>");
    int s_open_brace = grammar.symbolId("{");
    int s_close_brace = grammar.symbolId("}");
    nodeCreatorRegistry[s_initializer][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_initializer][{ s_open_brace, grammar.symbolId("<initializer_list>"), s_close_brace }] = initializer;

    int s_init_declarator = grammar.symbolId("<init_declarator>");
    nodeCreatorRegistry[s_init_declarator][{ s_declarator }] = initializedDeclarator;
    nodeCreatorRegistry[s_init_declarator][{ s_declarator, grammar.symbolId("="), s_initializer }] = initializedDeclaratorWithInitializer;

    int s_init_declarator_list = grammar.symbolId("<init_declarator_list>");
    nodeCreatorRegistry[s_init_declarator_list][{ s_init_declarator }] = initializedDeclaratorList;
    nodeCreatorRegistry[s_init_declarator_list][{ s_init_declarator_list, s_comma, s_init_declarator }] = addToInitializedDeclaratorList;

    int s_decl = grammar.symbolId("<decl>");
    int s_semicolon = grammar.symbolId(";");
    nodeCreatorRegistry[s_decl][{ s_decl_specs, s_init_declarator_list, s_semicolon }] = initializedDeclaration;
    nodeCreatorRegistry[s_decl][{ s_decl_specs, s_semicolon }] = declaration;

    int s_decl_list = grammar.symbolId("<decl_list>");
    nodeCreatorRegistry[s_decl_list][{ s_decl }] = declarationList;
    nodeCreatorRegistry[s_decl_list][{ s_decl_list, s_decl }] = addDeclarationToList;

    nodeCreatorRegistry[s_exp][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_exp][{ s_exp, s_comma, s_assignment }] = expressionList;

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier }] = typeQualifierList;
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier_list, s_type_qualifier }] = addTypeQualifierToList;

    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list }] = qualifiedPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*") }] = pointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list, s_pointer }] = qualifiedPointerToPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_pointer }] = pointerToPointer;

    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("&") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("*") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("+") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("-") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("~") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("!") }] = doNothing;

    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("*=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("/=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("%=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("+=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("-=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("<<=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId(">>=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("&=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("^=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("|=") }] = doNothing;

    int s_exp_stat = grammar.symbolId("<exp_stat>");
    nodeCreatorRegistry[s_exp_stat][{ s_exp, s_semicolon }] = expressionStatement;
    nodeCreatorRegistry[s_exp_stat][{ s_semicolon }] = emptyStatement;

    int s_matched = grammar.symbolId("<matched>");
    int s_unmatched = grammar.symbolId("<unmatched>");
    int s_stat = grammar.symbolId("<stat>");
    int s_compound_stat = grammar.symbolId("<compound_stat>");
    int s_jump_stat = grammar.symbolId("<jump_stat>");
    int s_if = grammar.symbolId("if");
    nodeCreatorRegistry[s_matched][{s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_matched }] = ifElseStatement;
    nodeCreatorRegistry[s_unmatched][{s_if, s_open_paren, s_exp, s_close_paren, s_stat }] = ifStatement;
    //nodeCreatorRegistry[s_matched][{ grammar.symbolId("switch"), s_open_paren, s_exp, s_close_paren, s_matched }] = switchStatement;
    //nodeCreatorRegistry[s_matched][{ grammar.symbolId("<labeled_stat_matched>") }] = labeledStatement; // ??
    nodeCreatorRegistry[s_matched][{ s_exp_stat }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_compound_stat }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_jump_stat }] = doNothing;

    nodeCreatorRegistry[s_stat][{ s_matched }] = doNothing;
    nodeCreatorRegistry[s_stat][{ s_unmatched }] = doNothing;

    int s_stat_list = grammar.symbolId("<stat_list>");
    nodeCreatorRegistry[s_stat_list][{ s_stat }] = statementList;
    nodeCreatorRegistry[s_stat_list][{ s_stat_list, s_stat }] = addToStatementList;

    int s_return = grammar.symbolId("return");
    //nodeCreatorRegistry[s_jump][{ grammar.symbolId("goto"), s_identifier, s_semicolon }] = gotoStatement;
    //nodeCreatorRegistry[s_jump][{ grammar.symbolId("continue"), s_semicolon }] = continueStatement;
    //nodeCreatorRegistry[s_jump][{ grammar.symbolId("break"), s_semicolon }] = breakStatement;
    nodeCreatorRegistry[s_jump_stat][{ s_return, s_exp, s_semicolon }] = returnExpressionStatement;
    nodeCreatorRegistry[s_jump_stat][{ s_return, s_semicolon }] = returnVoidStatement;

    nodeCreatorRegistry[s_argument_exp_list][{ s_assignment }] = createActualArgumentsList;
    nodeCreatorRegistry[s_argument_exp_list][{ s_argument_exp_list, s_comma, s_assignment }] = addToActualArgumentsList;

    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_decl_list, s_stat_list, s_close_brace}] = fullCompound;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_stat_list, s_close_brace}] = statementCompound;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_decl_list, s_close_brace}] = declarationCompound;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_close_brace}] = emptyCompound;

    int s_function_definition = grammar.symbolId("<function_definition>");
    nodeCreatorRegistry[s_function_definition][{ s_decl_specs, s_declarator, s_compound_stat }] = functionDefinition;
    nodeCreatorRegistry[s_function_definition][{ s_declarator, s_compound_stat }] = defaultReturnTypeFunctionDefinition;

    int s_external_decl = grammar.symbolId("<external_decl>");
    nodeCreatorRegistry[s_external_decl][{ s_function_definition }] = externalFunctionDefinition;
    nodeCreatorRegistry[s_external_decl][{ s_decl }] = externalDeclaration;

    int s_translation_unit = grammar.symbolId("<translation_unit>");
    nodeCreatorRegistry[s_translation_unit][{ s_external_decl }] = translationUnit;
    nodeCreatorRegistry[s_translation_unit][{ s_translation_unit, s_external_decl }] = addToTranslationUnit;

    int s_iteration_stat_matched = grammar.symbolId("<iteration_stat_matched>");
    int s_iteration_stat_unmatched = grammar.symbolId("<iteration_stat_unmatched>");
    nodeCreatorRegistry[s_matched][{ s_iteration_stat_matched }] = doNothing;
    nodeCreatorRegistry[s_unmatched][{ s_iteration_stat_unmatched }] = doNothing;

    int s_while = grammar.symbolId("while");
    int s_for = grammar.symbolId("for");
    nodeCreatorRegistry[s_iteration_stat_matched][{ s_while, s_open_paren, s_exp, s_close_paren, s_matched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_unmatched][{ s_while, s_open_paren, s_exp, s_close_paren, s_unmatched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_matched][{ s_for, s_open_paren, s_exp, s_semicolon, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }] = forLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_unmatched][{ s_for, s_open_paren, s_exp, s_semicolon, s_exp, s_semicolon, s_exp, s_close_paren, s_unmatched }] = forLoopStatement;
}

ContextualSyntaxNodeBuilder::~ContextualSyntaxNodeBuilder() = default;

void ContextualSyntaxNodeBuilder::updateContext(const parser::Production& production, AbstractSyntaxTreeBuilderContext& context) const {
    try {
        nodeCreatorRegistry.at(production.getDefiningSymbol()).at(production.producedSequence())(context);
    } catch (std::out_of_range& exception) {
        noCreatorDefined(production);
    }
}

void ContextualSyntaxNodeBuilder::noCreatorDefined(const parser::Production& production) const {
    throw std::runtime_error { "no AST creator defined for production `" + grammar->str(production) + "`" };
}

void ContextualSyntaxNodeBuilder::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStatement(std::make_unique<JumpStatement>(context.popTerminal()));
    context.popTerminal();
}

} /* namespace ast */
