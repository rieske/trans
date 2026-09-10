#include "CSNB_Internal.h"

#include "AbstractSyntaxTreeBuilderContext.h"
#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "AssignmentExpression.h"
#include "BitwiseExpression.h"
#include "ComparisonExpression.h"
#include "CompoundLiteral.h"
#include "ConditionalExpression.h"
#include "ConstantExpression.h"
#include "ExpressionList.h"
#include "FunctionCall.h"
#include "IdentifierExpression.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "MemberAccess.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "StringLiteralExpression.h"
#include "TypeCast.h"
#include "TypeNameExpression.h"
#include "UnaryExpression.h"
#include "types/IntegerConstant.h"
#include "types/Operator.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/IntegerLiteral.h"

#include <string>

namespace ast {

void parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
}

namespace {

type::Type integerLiteralType(const std::string& token) {
    util::IntegerLiteral lit;
    if (!util::parseIntegerLiteral(token, lit)) {
        return type::signedInteger();
    }
    return type::rankedLiteral(lit.value, lit.base, lit.uns, lit.lng).type;
}

} // namespace

void integerConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, integerLiteralType(constant.value), constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedInteger(), constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    const int size = util::floatingLiteralSizeBytes(constant.value);
    type::Type t = type::doubleFloating();
    if (size == 4) {
        t = type::floating();
    } else if (size == 16) {
        t = type::longDoubleFloating();
    }
    context.pushConstant( { constant.value, t, constant.context });
}

void identifierExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto identifier = context.popTerminal();
    type::IntegerConstant ice;
    const bool folded = context.environment().lookupInnermostEnumerator(identifier.value, ice);
    auto expr = std::make_unique<IdentifierExpression>(std::move(identifier.value), identifier.context);
    if (folded) {
        expr->setFoldedConstant(std::move(ice));
    }
    context.pushExpression(std::move(expr));
}

void constantExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<ConstantExpression>(context.popConstant()));
}

void stringLiteralExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto literal = context.popTerminal();
    context.pushExpression(std::make_unique<StringLiteralExpression>(std::move(literal.value), literal.context));
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
    auto member = context.popTerminal(); // id
    context.popTerminal(); // .
    auto base = context.popExpression();
    context.pushExpression(std::make_unique<MemberAccess>(
            std::move(base), member.value, false, member.context));
}

void pointeeMemberAccess(AbstractSyntaxTreeBuilderContext& context) {
    auto member = context.popTerminal(); // id
    context.popTerminal(); // ->
    auto base = context.popExpression();
    context.pushExpression(std::make_unique<MemberAccess>(
            std::move(base), member.value, true, member.context));
}

void postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PostfixExpression>(context.popExpression(), type::requireOp(type::incDecFromLexeme(context.popTerminal().value))));
}

void prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PrefixExpression>(type::requireOp(type::incDecFromLexeme(context.popTerminal().value)), context.popExpression()));
}

void unaryExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<UnaryExpression>(type::requireOp(type::unaryOpFromLexeme(context.popTerminal().value)), context.popExpression()));
}

void sizeofExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // sizeof
    context.pushExpression(std::make_unique<UnaryExpression>(
            type::UnaryOp::Sizeof, context.popExpression()));
}

void typeofTypeName(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // typeof
    auto typeSpec = context.popTypeSpecifier();
    typeSpec.dropSpelling();
    context.pushTypeSpecifier(std::move(typeSpec));
}

void typeofExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto expr = context.popExpression();
    context.popTerminal(); // (
    context.popTerminal(); // typeof
    if (auto parsed = context.environment().typeOf(*expr)) {
        context.pushTypeSpecifier(TypeSpecifier { *parsed, "" });
        return;
    }
    context.pushTypeSpecifier(TypeSpecifier { std::move(expr) });
}

void genericAssociationTyped(AbstractSyntaxTreeBuilderContext& context) {
    auto expr = context.popExpression();
    context.popTerminal(); // :
    context.pushGenericAssociation(GenericAssociation { context.popTypeSpecifier(), std::move(expr) });
}

void genericAssociationDefault(AbstractSyntaxTreeBuilderContext& context) {
    auto expr = context.popExpression();
    context.popTerminal(); // :
    context.popTerminal(); // default
    context.pushGenericAssociation(GenericAssociation { std::nullopt, std::move(expr) });
}

void genericAssocListFirst(AbstractSyntaxTreeBuilderContext& context) {
    context.newGenericAssocList(context.popGenericAssociation());
}

void genericAssocListAppend(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.addGenericAssociation(context.popGenericAssociation());
}

void genericSelection(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto associations = context.popGenericAssocList();
    context.popTerminal(); // ,
    auto controlling = context.popExpression();
    context.popTerminal(); // (
    auto kw = context.popTerminal(); // _Generic
    context.pushExpression(std::make_unique<GenericSelection>(
            kw.context, std::move(controlling), std::move(associations)));
}

void nullptrExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto kw = context.popTerminal();
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { "0", type::pointer(type::voidType()), kw.context }));
}

namespace {

void boolConstantExpression(AbstractSyntaxTreeBuilderContext& context, const char* digits) {
    auto kw = context.popTerminal();
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { digits, type::boolean(), kw.context }));
}

} // namespace

void trueExpression(AbstractSyntaxTreeBuilderContext& context) {
    boolConstantExpression(context, "1");
}

void falseExpression(AbstractSyntaxTreeBuilderContext& context) {
    boolConstantExpression(context, "0");
}

void sizeofTypeExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    // type_name left a TypeSpecifier for simple types (int, char, long, pointers via abstract decl later).
    auto typeSpec = context.popTypeSpecifier();
    auto sizeofKw = context.popTerminal(); // sizeof
    if (!typeSpec.resolveTypeofAtParseTime(context.environment())) {
        context.error(sizeofKw.context, "cannot determine type of typeof operand");
        return;
    }
    if (typeSpec.enumerators().empty()) {
        if (auto bytes = type::sizeofObject(typeSpec.getType(), context.environment().gnuExtensions())) {
            context.pushExpression(std::make_unique<ConstantExpression>(
                    Constant { std::to_string(*bytes), type::signedInteger(), sizeofKw.context }));
            return;
        }
    }
    context.pushExpression(std::make_unique<UnaryExpression>(
            type::UnaryOp::Sizeof,
            std::make_unique<TypeNameExpression>(std::move(typeSpec), sizeofKw.context)));
}

void typeNameWithAbstractDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    auto specs = popResolvedSpecQualifiers(context);
    if (context.failed()) {
        return;
    }
    auto typeSpec = specs.toTypeSpecifier();
    typeSpec.deferAbstractDeclarator(std::move(declarator));
    context.pushTypeSpecifier(std::move(typeSpec));
}

void typeCast(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto castExpression = context.popExpression();
    context.popTerminal(); // (
    auto typeSpec = context.popTypeSpecifier();
    context.pushExpression(std::make_unique<TypeCast>(std::move(typeSpec), std::move(castExpression)));
}

namespace {

void pushCompoundLiteral(AbstractSyntaxTreeBuilderContext& context, bool trailingComma) {
    context.popTerminal(); // }
    if (trailingComma) {
        context.popTerminal(); // ,
    }
    auto elements = context.popInitializerList();
    context.popTerminal(); // {
    context.popTerminal(); // )
    context.popTerminal(); // (
    auto typeSpec = context.popTypeSpecifier();
    context.pushExpression(std::make_unique<CompoundLiteral>(std::move(typeSpec),
            std::make_unique<InitializerListExpression>(std::move(elements))));
}

} // namespace

void compoundLiteral(AbstractSyntaxTreeBuilderContext& context) {
    pushCompoundLiteral(context, false);
}

void compoundLiteralTrailingComma(AbstractSyntaxTreeBuilderContext& context) {
    pushCompoundLiteral(context, true);
}

void arithmeticExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto arithmeticOperator = type::requireOp(type::arithmeticOpFromLexeme(context.popTerminal().value));
    context.pushExpression(std::make_unique<ArithmeticExpression>(std::move(leftHandSide), arithmeticOperator, std::move(rightHandSide)));
}

void shiftExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto additionExpression = context.popExpression();
    auto shiftExpression = context.popExpression();
    auto shiftOperator = type::requireOp(type::shiftOpFromLexeme(context.popTerminal().value));
    context.pushExpression(std::make_unique<ShiftExpression>(std::move(shiftExpression), shiftOperator, std::move(additionExpression)));
}

void relationalExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto comparisonOperator = type::requireOp(type::comparisonOpFromLexeme(context.popTerminal().value));
    context.pushExpression(std::make_unique<ComparisonExpression>(std::move(leftHandSide), comparisonOperator, std::move(rightHandSide)));
}

void bitwiseExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto bitwiseOperator = type::requireOp(type::bitwiseOpFromLexeme(context.popTerminal().value));
    context.pushExpression(std::make_unique<BitwiseExpression>(std::move(leftHandSide), bitwiseOperator, std::move(rightHandSide)));
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
    // Production: <logical_or_exp> '?' <exp> ':' <conditional_exp>
    // Expressions reduce LIFO: false arm, true arm, then condition; then '?' / ':'.
    context.popTerminal(); // :
    context.popTerminal(); // ?
    auto falseExpression = context.popExpression();
    auto trueExpression = context.popExpression();
    auto condition = context.popExpression();
    context.pushExpression(std::make_unique<ConditionalExpression>(
            std::move(condition), std::move(trueExpression), std::move(falseExpression)));
}

void assignmentExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto assignmentOperator = type::requireOp(type::assignOpFromLexeme(context.popTerminal().value));
    context.pushExpression(
            std::make_unique<AssignmentExpression>(std::move(leftHandSide), assignmentOperator, std::move(rightHandSide)));
}

void expressionList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<ExpressionList>(std::move(leftHandSide), std::move(rightHandSide)));
}

void createActualArgumentsList(AbstractSyntaxTreeBuilderContext& context) {
    context.newActualArgumentsList(context.popExpression());
}

void addToActualArgumentsList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.addToActualArgumentsList(context.popExpression());
}


} // namespace ast
