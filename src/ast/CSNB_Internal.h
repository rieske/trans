#ifndef CSNB_INTERNAL_H_
#define CSNB_INTERNAL_H_

#include "ContextualSyntaxNodeBuilder.h"
#include "DeclarationSpecifiers.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclarator.h"
#include "AssignmentExpression.h"
#include "BitwiseExpression.h"
#include "Block.h"
#include "ComparisonExpression.h"
#include "ConditionalExpression.h"
#include "Constant.h"
#include "ConstantExpression.h"
#include "MemberAccess.h"
#include "InitializerListExpression.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FunctionCall.h"
#include "FunctionDefinition.h"
#include "Identifier.h"
#include "IdentifierExpression.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "JumpStatement.h"
#include "GotoStatement.h"
#include "LabeledStatement.h"
#include "SwitchStatement.h"
#include "CaseLabel.h"
#include "DefaultLabel.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "Operator.h"
#include "ParenthesizedDeclarator.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ReturnStatement.h"
#include "VoidReturnStatement.h"
#include "ShiftExpression.h"
#include "TypeCast.h"
#include "CompoundLiteral.h"
#include "UnaryExpression.h"
#include "WhileLoopHeader.h"
#include "DoWhileLoopHeader.h"
#include "ast/StringLiteralExpression.h"
#include "GenericSelection.h"

namespace ast {

std::function<void(AbstractSyntaxTreeBuilderContext&)> notImplementedYet(const char* feature);

void abstractArrayDeclarator(AbstractSyntaxTreeBuilderContext& context);
void abstractArrayDeclaratorQualified(AbstractSyntaxTreeBuilderContext& context);
void abstractArrayOnlySized(AbstractSyntaxTreeBuilderContext& context);
void abstractArrayOnlyUnsized(AbstractSyntaxTreeBuilderContext& context);
void abstractArrayOnlyQualifiedSized(AbstractSyntaxTreeBuilderContext& context);
void abstractArrayOnlyQualifiedUnsized(AbstractSyntaxTreeBuilderContext& context);
void abstractFuncOnly(AbstractSyntaxTreeBuilderContext& context);
void abstractNoargOnly(AbstractSyntaxTreeBuilderContext& context);
void abstractParameterDeclaration(AbstractSyntaxTreeBuilderContext& context);
void abstractPointerDeclarator(AbstractSyntaxTreeBuilderContext& context);
void addDeclarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context);
void addDeclarationToList(AbstractSyntaxTreeBuilderContext& context);
void addDeclarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context);
void addDeclarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context);
void addFormalArgument(AbstractSyntaxTreeBuilderContext& context);
void addToActualArgumentsList(AbstractSyntaxTreeBuilderContext& context);
void addToInitializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context);
void addToStatementList(AbstractSyntaxTreeBuilderContext& context);
void addToTranslationUnit(AbstractSyntaxTreeBuilderContext& context);
void addTypeQualifierToList(AbstractSyntaxTreeBuilderContext& context);
void arithmeticExpression(AbstractSyntaxTreeBuilderContext& context);
void arrayAccess(AbstractSyntaxTreeBuilderContext& context);
void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context);
void arrayDeclaratorQualified(AbstractSyntaxTreeBuilderContext& context);
void assignmentExpression(AbstractSyntaxTreeBuilderContext& context);
void bitwiseExpression(AbstractSyntaxTreeBuilderContext& context);
void boolType(AbstractSyntaxTreeBuilderContext& context);
void blockItemDeclaration(AbstractSyntaxTreeBuilderContext& context);
void blockItemListCompound(AbstractSyntaxTreeBuilderContext& context);
void braceInitializer(AbstractSyntaxTreeBuilderContext& context);
void braceInitializerTrailingComma(AbstractSyntaxTreeBuilderContext& context);
void arrayDesignator(AbstractSyntaxTreeBuilderContext& context);
void caseLabel(AbstractSyntaxTreeBuilderContext& context);
void characterConstant(AbstractSyntaxTreeBuilderContext& context);
void characterType(AbstractSyntaxTreeBuilderContext& context);
DeclarationSpecifiers popResolvedSpecQualifiers(AbstractSyntaxTreeBuilderContext& context);
void specQualifierListTypeName(AbstractSyntaxTreeBuilderContext& context);
void conditionalExpression(AbstractSyntaxTreeBuilderContext& context);
void complexType(AbstractSyntaxTreeBuilderContext& context);
void constQualifier(AbstractSyntaxTreeBuilderContext& context);
void autoStorageClass(AbstractSyntaxTreeBuilderContext& context);
void registerStorageClass(AbstractSyntaxTreeBuilderContext& context);
void restrictQualifier(AbstractSyntaxTreeBuilderContext& context);
void staticStorageClass(AbstractSyntaxTreeBuilderContext& context);
void externStorageClass(AbstractSyntaxTreeBuilderContext& context);
void typedefStorageClass(AbstractSyntaxTreeBuilderContext& context);
void constantExpression(AbstractSyntaxTreeBuilderContext& context);
void createActualArgumentsList(AbstractSyntaxTreeBuilderContext& context);
void declaration(AbstractSyntaxTreeBuilderContext& context);
void declarationList(AbstractSyntaxTreeBuilderContext& context);
void declarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context);
void declarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context);
void declarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context);
void declarator(AbstractSyntaxTreeBuilderContext& context);
void defaultLabel(AbstractSyntaxTreeBuilderContext& context);
void defaultReturnTypeFunctionDefinition(AbstractSyntaxTreeBuilderContext& context);
void designation(AbstractSyntaxTreeBuilderContext& context);
void designatedInitializerListAppend(AbstractSyntaxTreeBuilderContext& context);
void designatedInitializerListFirst(AbstractSyntaxTreeBuilderContext& context);
void designatorListAppend(AbstractSyntaxTreeBuilderContext& context);
void designatorListSingle(AbstractSyntaxTreeBuilderContext& context);
void directMemberAccess(AbstractSyntaxTreeBuilderContext& context);
void doNothing(AbstractSyntaxTreeBuilderContext& context);
void doWhileLoopStatement(AbstractSyntaxTreeBuilderContext& context);
void doubleType(AbstractSyntaxTreeBuilderContext& context);
void emptyCompound(AbstractSyntaxTreeBuilderContext& context);
void emptyStatement(AbstractSyntaxTreeBuilderContext& context);
void enumType(AbstractSyntaxTreeBuilderContext& context);
void enumerationConstant(AbstractSyntaxTreeBuilderContext& context);
void expressionList(AbstractSyntaxTreeBuilderContext& context);
void expressionStatement(AbstractSyntaxTreeBuilderContext& context);
void externalDeclaration(AbstractSyntaxTreeBuilderContext& context);
void externalFunctionDefinition(AbstractSyntaxTreeBuilderContext& context);
void floatConstant(AbstractSyntaxTreeBuilderContext& context);
void floatType(AbstractSyntaxTreeBuilderContext& context);
void functionSpecifier(AbstractSyntaxTreeBuilderContext& context);
void functionSpecifierOnly(AbstractSyntaxTreeBuilderContext& context);
void formalArguments(AbstractSyntaxTreeBuilderContext& context);
void formalArgumentsDeclaration(AbstractSyntaxTreeBuilderContext& context);
void formalArgumentsWithVararg(AbstractSyntaxTreeBuilderContext& context);
void functionCall(AbstractSyntaxTreeBuilderContext& context);
void genericAssociationDefault(AbstractSyntaxTreeBuilderContext& context);
void genericAssociationTyped(AbstractSyntaxTreeBuilderContext& context);
void genericAssocListAppend(AbstractSyntaxTreeBuilderContext& context);
void genericAssocListFirst(AbstractSyntaxTreeBuilderContext& context);
void genericSelection(AbstractSyntaxTreeBuilderContext& context);
void functionDeclarator(AbstractSyntaxTreeBuilderContext& context);
void functionDefinition(AbstractSyntaxTreeBuilderContext& context);
void gotoStatement(AbstractSyntaxTreeBuilderContext& context);
void identifierDeclarator(AbstractSyntaxTreeBuilderContext& context);
void identifierExpression(AbstractSyntaxTreeBuilderContext& context);
void ifElseStatement(AbstractSyntaxTreeBuilderContext& context);
void ifStatement(AbstractSyntaxTreeBuilderContext& context);
void initializedDeclaration(AbstractSyntaxTreeBuilderContext& context);
void initializedDeclarator(AbstractSyntaxTreeBuilderContext& context);
void initializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context);
void initializedDeclaratorWithInitializer(AbstractSyntaxTreeBuilderContext& context);
void initializerListAppend(AbstractSyntaxTreeBuilderContext& context);
void initializerListFirst(AbstractSyntaxTreeBuilderContext& context);
void integerConstant(AbstractSyntaxTreeBuilderContext& context);
void integerType(AbstractSyntaxTreeBuilderContext& context);
void logicalAndExpression(AbstractSyntaxTreeBuilderContext& context);
void logicalOrExpression(AbstractSyntaxTreeBuilderContext& context);
void longType(AbstractSyntaxTreeBuilderContext& context);
void memberDesignator(AbstractSyntaxTreeBuilderContext& context);
void namedLabel(AbstractSyntaxTreeBuilderContext& context);
void noargFunctionCall(AbstractSyntaxTreeBuilderContext& context);
void noargFunctionDeclarator(AbstractSyntaxTreeBuilderContext& context);
void parameterBaseTypeDeclaration(AbstractSyntaxTreeBuilderContext& context);
void parameterDeclaration(AbstractSyntaxTreeBuilderContext& context);
void parenthesizedDeclarator(AbstractSyntaxTreeBuilderContext& context);
void parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context);
void pointeeMemberAccess(AbstractSyntaxTreeBuilderContext& context);
void pointer(AbstractSyntaxTreeBuilderContext& context);
void pointerToDeclarator(AbstractSyntaxTreeBuilderContext& context);
void pointerToPointer(AbstractSyntaxTreeBuilderContext& context);
void postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context);
void prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context);
void qualifiedPointer(AbstractSyntaxTreeBuilderContext& context);
void qualifiedPointerToPointer(AbstractSyntaxTreeBuilderContext& context);
void relationalExpression(AbstractSyntaxTreeBuilderContext& context);
void returnExpressionStatement(AbstractSyntaxTreeBuilderContext& context);
void returnVoidStatement(AbstractSyntaxTreeBuilderContext& context);
void shiftExpression(AbstractSyntaxTreeBuilderContext& context);
void shortType(AbstractSyntaxTreeBuilderContext& context);
void signedType(AbstractSyntaxTreeBuilderContext& context);
void sizeofExpression(AbstractSyntaxTreeBuilderContext& context);
void sizeofTypeExpression(AbstractSyntaxTreeBuilderContext& context);
void statementList(AbstractSyntaxTreeBuilderContext& context);
void stringLiteralExpression(AbstractSyntaxTreeBuilderContext& context);
void structOrUnionType(AbstractSyntaxTreeBuilderContext& context);
void switchStatement(AbstractSyntaxTreeBuilderContext& context);
void translationUnit(AbstractSyntaxTreeBuilderContext& context);
void typeCast(AbstractSyntaxTreeBuilderContext& context);
void compoundLiteral(AbstractSyntaxTreeBuilderContext& context);
void compoundLiteralTrailingComma(AbstractSyntaxTreeBuilderContext& context);
void typeNameWithAbstractDeclarator(AbstractSyntaxTreeBuilderContext& context);
void typeQualifierList(AbstractSyntaxTreeBuilderContext& context);
void typedefName(AbstractSyntaxTreeBuilderContext& context);
void typeofExpression(AbstractSyntaxTreeBuilderContext& context);
void typeofTypeName(AbstractSyntaxTreeBuilderContext& context);
void nullptrExpression(AbstractSyntaxTreeBuilderContext& context);
void falseExpression(AbstractSyntaxTreeBuilderContext& context);
void trueExpression(AbstractSyntaxTreeBuilderContext& context);
void unaryExpression(AbstractSyntaxTreeBuilderContext& context);
void unsignedType(AbstractSyntaxTreeBuilderContext& context);
void voidType(AbstractSyntaxTreeBuilderContext& context);
void volatileQualifier(AbstractSyntaxTreeBuilderContext& context);
void whileLoopStatement(AbstractSyntaxTreeBuilderContext& context);

} // namespace ast

#endif // CSNB_INTERNAL_H_
