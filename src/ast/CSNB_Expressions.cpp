#include "CSNB_Internal.h"

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "AssignmentExpression.h"
#include "BitwiseExpression.h"
#include "ComparisonExpression.h"
#include "ConditionalExpression.h"
#include "ConstantExpression.h"
#include "ExpressionList.h"
#include "FunctionCall.h"
#include "IdentifierExpression.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "MemberAccess.h"
#include "Operator.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "ast/StringLiteralExpression.h"
#include "types/Type.h"

namespace ast {
namespace csnb {

namespace {

void parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
}
void integerConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    // C integer suffixes: U/u -> unsigned; L/l with U -> unsigned long; bare L -> long.
    // UL constants must be unsigned so `1 > maxUL - size` uses unsigned compare
    // (git unsigned_add_overflows / xmallocz).
    type::Type constType = type::signedInteger();
    const std::string& text = constant.value;
    bool hasU = false;
    bool hasL = false;
    for (char c : text) {
        if (c == 'u' || c == 'U') {
            hasU = true;
        } else if (c == 'l' || c == 'L') {
            hasL = true;
        }
    }
    if (hasU && hasL) {
        constType = type::unsignedLong();
    } else if (hasU) {
        constType = type::unsignedInteger();
    } else if (hasL) {
        constType = type::signedLong();
    } else {
        // C99 6.4.4.1 unsuffixed: pick the first type that can represent the value.
        // Decimal: int -> long -> long long. Octal/hex: int -> unsigned int -> long -> ...
        // (git jw_object_intmax(..., 0xffffffff) must be unsigned int, not signed -1).
        std::string digits = text;
        while (!digits.empty()) {
            char c = digits.back();
            if (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                digits.pop_back();
            } else {
                break;
            }
        }
        unsigned long long mag = 0;
        bool parsed = false;
        try {
            if (!digits.empty()) {
                mag = std::stoull(digits, nullptr, 0);
                parsed = true;
            }
        } catch (...) {
            parsed = false;
        }
        if (parsed) {
            const bool isDecimal = !(digits.size() >= 2 && digits[0] == '0');
            constexpr unsigned long long INT_MAX_U = 2147483647ull;
            constexpr unsigned long long UINT_MAX_U = 4294967295ull;
            constexpr unsigned long long LONG_MAX_U = 9223372036854775807ull;
            if (mag <= INT_MAX_U) {
                constType = type::signedInteger();
            } else if (!isDecimal && mag <= UINT_MAX_U) {
                constType = type::unsignedInteger();
            } else if (mag <= LONG_MAX_U) {
                constType = type::signedLong();
            } else if (!isDecimal) {
                constType = type::unsignedLong();
            } else {
                // Decimal above LONG_MAX: still long long bits as unsigned long storage.
                constType = type::unsignedLong();
            }
        }
    }
    context.pushConstant( { constant.value, constType, constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedCharacter(), constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    // Default float_const lexemes to double (C: unsuffixed floating constants are double).
    context.pushConstant( { constant.value, type::doubleFloating(), constant.context });
}

void enumerationConstant(AbstractSyntaxTreeBuilderContext& context) {
    // Scanner emits enum constants as plain identifiers; this token path is unused.
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedInteger(), constant.context });
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
    auto member = context.popTerminal();
    context.popTerminal();
    auto base = context.popExpression();
    context.pushExpression(std::make_unique<MemberAccess>(std::move(base), member.value, false, member.context));
}

void pointeeMemberAccess(AbstractSyntaxTreeBuilderContext& context) {
    auto member = context.popTerminal();
    context.popTerminal();
    auto base = context.popExpression();
    context.pushExpression(std::make_unique<MemberAccess>(std::move(base), member.value, true, member.context));
}

void postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PostfixExpression>(
            context.popExpression(), Operator::makeUnary(context.popTerminal().type)));
}

void prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PrefixExpression>(
            Operator::makeUnary(context.popTerminal().value), context.popExpression()));
}

void unaryExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<UnaryExpression>(
            Operator::makeUnary(context.popTerminal().value), context.popExpression()));
}

void sizeofExpression(AbstractSyntaxTreeBuilderContext& context) {
    // sizeof <unary_exp> - fold in semantic analysis via UnaryExpression "sizeof".
    context.popTerminal(); // sizeof
    context.pushExpression(std::make_unique<UnaryExpression>(
            Operator::makeUnary("sizeof"), context.popExpression()));
}

void sizeofTypeExpression(AbstractSyntaxTreeBuilderContext& context) {
    // sizeof ( <type_name> ) - type_name leaves a TypeSpecifier on the stack for simple types.
    context.popTerminal(); // )
    context.popTerminal(); // (
    auto typeSpec = context.popTypeSpecifier();
    // Discard typeof operand if this type_name was __typeof__(expr).
    if (typeSpec.getName() == "__typeof__" && context.hasTypeofOperand()) {
        (void)context.popTypeofOperand();
    }
    auto sizeofKw = context.popTerminal(); // sizeof
    int size = typeSpec.getType().getSize();
    if (size < 0) {
        size = 0;
    }
    // size_t is unsigned; signed sizeof made `1 > max - size` use JG (git xmallocz).
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { std::to_string(size), type::unsignedLong(), sizeofKw.context }));
}
void typeCast(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto castExpression = context.popExpression();
    auto typeSpec = context.popTypeSpecifier();
    context.popTerminal(); // (
    context.pushExpression(std::make_unique<TypeCast>(typeSpec, std::move(castExpression)));
}
void genericAssociationType(AbstractSyntaxTreeBuilderContext& context) {
    // type_name ':' assignment_exp
    auto expr = context.popExpression();
    context.popTerminal(); // :
    auto typeSpec = context.popTypeSpecifier();
    discardTypeofIfPresent(context, typeSpec);
    AbstractSyntaxTreeBuilderContext::GenericAssociation assoc;
    assoc.isDefault = false;
    assoc.expression = std::move(expr);
    context.pushGenericAssociation(std::move(assoc));
}

void genericAssociationDefault(AbstractSyntaxTreeBuilderContext& context) {
    // default ':' assignment_exp
    auto expr = context.popExpression();
    context.popTerminal(); // :
    context.popTerminal(); // default
    AbstractSyntaxTreeBuilderContext::GenericAssociation assoc;
    assoc.isDefault = true;
    assoc.expression = std::move(expr);
    context.pushGenericAssociation(std::move(assoc));
}

void genericAssocListSingle(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<AbstractSyntaxTreeBuilderContext::GenericAssociation> list;
    list.push_back(context.popGenericAssociation());
    context.pushGenericAssociationList(std::move(list));
}

void genericAssocListAppend(AbstractSyntaxTreeBuilderContext& context) {
    auto assoc = context.popGenericAssociation();
    context.popTerminal(); // ,
    auto list = context.popGenericAssociationList();
    list.push_back(std::move(assoc));
    context.pushGenericAssociationList(std::move(list));
}
void genericPrimaryExpression(AbstractSyntaxTreeBuilderContext& context) {
    // Product contract (matches prior sanitizer): default association if present,
    // else the last association. Controlling expression is type-checked only by
    // being reduced; its value is discarded.
    context.popTerminal(); // )
    auto list = context.popGenericAssociationList();
    context.popTerminal(); // ,
    (void)context.popExpression(); // controlling expression
    context.popTerminal(); // (
    context.popTerminal(); // _Generic

    std::unique_ptr<Expression> chosen;
    std::unique_ptr<Expression> last;
    for (auto& assoc : list) {
        if (assoc.isDefault) {
            chosen = std::move(assoc.expression);
        } else {
            last = std::move(assoc.expression);
        }
    }
    if (!chosen) {
        chosen = std::move(last);
    }
    if (!chosen) {
        // Empty association list should not parse; push 0 as a safe fallback.
        context.pushExpression(std::make_unique<ConstantExpression>(
                Constant { "0", type::signedInteger(), translation_unit::Context { "", 0 } }));
        return;
    }
    context.pushExpression(std::move(chosen));
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
    auto assignmentOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(
            std::make_unique<AssignmentExpression>(std::move(leftHandSide), std::move(assignmentOperator), std::move(rightHandSide)));
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

} // namespace


void registerExpressionProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
    int s_identifier = grammar.symbolId("id");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_open_bracket = grammar.symbolId("[");
    int s_close_bracket = grammar.symbolId("]");
    int s_comma = grammar.symbolId(",");
    int s_exp = grammar.symbolId("<exp>");
    int s_constant = grammar.symbolId("<const>");
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("int_const") }] = integerConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("char_const") }] = characterConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("float_const") }] = floatConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("enumeration_const") }] = enumerationConstant;

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
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("."), s_identifier }] = directMemberAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("->"), s_identifier }] = pointeeMemberAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("++") }] = postfixIncrementDecrement;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("--") }] = postfixIncrementDecrement;

    int s_cast_exp = grammar.symbolId("<cast_exp>");
    int s_unary_exp = grammar.symbolId("<unary_exp>");
    int s_unary_operator = grammar.symbolId("<unary_operator>");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_assignment = grammar.symbolId("<assignment_exp>");
    int s_generic_assoc_list = grammar.symbolId("<generic_assoc_list>");
    int s_generic_association = grammar.symbolId("<generic_association>");
    nodeCreatorRegistry[s_primary_exp][{
            grammar.symbolId("_Generic"), s_open_paren, s_assignment, grammar.symbolId(","),
            s_generic_assoc_list, s_close_paren }] = genericPrimaryExpression;
    nodeCreatorRegistry[s_generic_assoc_list][{ s_generic_association }] = genericAssocListSingle;
    nodeCreatorRegistry[s_generic_assoc_list][{
            s_generic_assoc_list, grammar.symbolId(","), s_generic_association }] = genericAssocListAppend;
    nodeCreatorRegistry[s_generic_association][{
            s_type_name, grammar.symbolId(":"), s_assignment }] = genericAssociationType;
    nodeCreatorRegistry[s_generic_association][{
            grammar.symbolId("default"), grammar.symbolId(":"), s_assignment }] = genericAssociationDefault;

    nodeCreatorRegistry[s_unary_exp][{ s_postfix_exp }] = doNothing;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("++"), s_unary_exp }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("--"), s_unary_exp }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unary_exp][{ s_unary_operator, s_cast_exp }] = unaryExpression;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("sizeof"), s_unary_exp }] = sizeofExpression;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("sizeof"), s_open_paren, s_type_name, s_close_paren }] = sizeofTypeExpression;

    nodeCreatorRegistry[s_cast_exp][{ s_unary_exp }] = doNothing;
    nodeCreatorRegistry[s_cast_exp][{ s_open_paren, s_type_name, s_close_paren, s_cast_exp }] = typeCast;

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

    int s_const_exp = grammar.symbolId("<const_exp>");
    nodeCreatorRegistry[s_const_exp][{ s_conditional_exp }] = doNothing;

    int s_assignment_operator = grammar.symbolId("<assignment_operator>");
    nodeCreatorRegistry[s_assignment][{ s_conditional_exp }] = doNothing;
    nodeCreatorRegistry[s_assignment][{ s_unary_exp, s_assignment_operator, s_assignment }] = assignmentExpression;

    nodeCreatorRegistry[s_exp][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_exp][{ s_exp, s_comma, s_assignment }] = expressionList;

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

    nodeCreatorRegistry[s_argument_exp_list][{ s_assignment }] = createActualArgumentsList;
    nodeCreatorRegistry[s_argument_exp_list][{ s_argument_exp_list, s_comma, s_assignment }] = addToActualArgumentsList;
}

} // namespace csnb
} // namespace ast
