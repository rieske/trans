#include "GnuExtensions.h"

#include "AbstractSyntaxTreeBuilder.h"
#include "Block.h"
#include "Constant.h"
#include "ConstantExpression.h"
#include "FunctionCall.h"
#include "IdentifierExpression.h"
#include "Operator.h"
#include "StatementExpression.h"
#include "UnaryExpression.h"
#include "parser/LR1Parser.h"
#include "parser/ParsingTable.h"
#include "parser/TokenStream.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace ast {

namespace {

// Exhaustive: new OffsetofStatus values fail to compile under -Wswitch-enum.
[[noreturn]] void failOffsetof(const translation_unit::Context& context, type::OffsetofStatus status,
        const std::string& member) {
    const std::string prefix = translation_unit::to_string(context) + ": error: ";
    switch (status) {
    case type::OffsetofStatus::Incomplete:
        throw std::runtime_error { prefix + "offsetof on incomplete type" };
    case type::OffsetofStatus::Missing:
        throw std::runtime_error {
                prefix + "no member named ‘" + member + "’ in structure or union" };
    case type::OffsetofStatus::BitField:
        throw std::runtime_error {
                prefix + "cannot compute offset of bit-field ‘" + member + "’" };
    case type::OffsetofStatus::Ok:
        break;
    }
    throw std::logic_error { "unreachable OffsetofStatus" };
}

bool isInt128Lexeme(const std::string& lexeme) {
    return lexeme == "__int128" || lexeme == "__int128_t" || lexeme == "__uint128_t";
}

bool isRealImagLexeme(const std::string& lexeme) {
    return lexeme == "__real__" || lexeme == "__imag__";
}

bool runNestedParse(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& nestedStream,
        const parser::ParsingTable& table, parser::ParseExtensions* extensions,
        parser::LrStop stop) {
    return parser::runLrParse(table, nestedStream, nested, extensions, stop) == parser::LrFinish::Stopped
            && !nested.hasError();
}

// Shared dummy `int __gnu_x =` prefix for assignment/cast expression subparses.
struct DummyInitPrefix {
    scanner::Token tokens[3];

    explicit DummyInitPrefix(const translation_unit::Context& ctx) :
            tokens {
                    { "int", "int", ctx },
                    { "id", "__gnu_x", ctx },
                    { "=", "=", ctx },
            } {
    }

    const scanner::Token* data() const { return tokens; }
    static constexpr std::size_t size() { return 3; }
};

} // namespace

void GnuExtensions::installTypes(scanner::LexicalSession& session) const {
    session.typedefs.add("__builtin_va_list", type::builtinVaListType());
}

std::optional<std::size_t> GnuExtensions::tryGoto(std::size_t state, parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable) {
    const scanner::Token current = tokenStream.getCurrentToken();
    if (current.id == "(" && tokenStream.peek().id == "{") {
        const auto primary = parsingTable.getGrammar()->trySymbolId("<primary_exp>");
        if (!primary) {
            return std::nullopt;
        }
        return parsingTable.tryGoTo(state, *primary);
    }
    if (current.id == "id" && isInt128Lexeme(current.lexeme)) {
        const auto typeSpec = parsingTable.getGrammar()->trySymbolId("<type_spec>");
        if (!typeSpec) {
            return std::nullopt;
        }
        return parsingTable.tryGoTo(state, *typeSpec);
    }
    if (current.id == "id"
            && (current.lexeme == "__builtin_va_arg"
                    || current.lexeme == "__builtin_types_compatible_p"
                    || current.lexeme == "__builtin_offsetof"
                    || isRealImagLexeme(current.lexeme))) {
        const auto unary = parsingTable.getGrammar()->trySymbolId("<unary_exp>");
        if (!unary) {
            return std::nullopt;
        }
        return parsingTable.tryGoTo(state, *unary);
    }
    return std::nullopt;
}

bool GnuExtensions::accept(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
        parser::SyntaxTreeBuilder& syntaxTreeBuilder) {
    auto& builder = static_cast<AbstractSyntaxTreeBuilder&>(syntaxTreeBuilder);
    const scanner::Token current = tokenStream.getCurrentToken();
    if (current.id == "(") {
        return acceptStatementPrimary(tokenStream, parsingTable, builder);
    }
    if (current.id == "id" && isInt128Lexeme(current.lexeme)) {
        return acceptInt128(tokenStream, builder);
    }
    if (current.id == "id") {
        return acceptRealImag(tokenStream, parsingTable, builder)
                || acceptVaArg(tokenStream, parsingTable, builder)
                || acceptTypesCompatibleP(tokenStream, parsingTable, builder)
                || acceptOffsetof(tokenStream, parsingTable, builder);
    }
    return false;
}

bool GnuExtensions::isTypeExtensionToken(const scanner::Token& token) const {
    return token.id == "id" && isInt128Lexeme(token.lexeme);
}

bool GnuExtensions::consumeUntilLookahead(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& outer,
        const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
        int stopSymbol, const std::string& stopLookahead, const std::string& presentStopAs) {
    const translation_unit::Context ctx = outer.getCurrentToken().context;
    scanner::LexicalSession& session = nested.session();
    std::size_t prefixIndex = 0;
    int depth = 0;
    bool live = false;
    const std::string& innerLookahead = presentStopAs.empty() ? stopLookahead : presentStopAs;

    parser::TokenStream nestedStream { [&]() {
        if (prefixIndex < prefixCount) {
            live = false;
            return prefix[prefixIndex++];
        }
        live = true;
        scanner::Token token = outer.getCurrentToken();
        if (depth == 0 && token.id == stopLookahead) {
            if (!presentStopAs.empty()) {
                return scanner::Token { presentStopAs, presentStopAs, token.context };
            }
            return token;
        }
        if (token.id == "(" || token.id == "[" || token.id == "{") {
            ++depth;
        } else if (token.id == ")" || token.id == "]" || token.id == "}") {
            --depth;
        }
        return outer.takeRaw();
    }, session };

    return runNestedParse(nested, nestedStream, table, this,
            parser::LrStop::untilLookahead(stopSymbol, innerLookahead, &live));
}

bool GnuExtensions::consumeUntilComplete(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& outer,
        const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
        int stopSymbol) {
    scanner::LexicalSession& session = nested.session();
    std::size_t prefixIndex = 0;
    bool live = false;
    // Yield outer tokens by peek; takeRaw only when the nested stream advances.
    // Stops leave the outer lookahead untouched.
    bool holdOuter = false;

    parser::TokenStream nestedStream { [&]() {
        if (prefixIndex < prefixCount) {
            live = false;
            holdOuter = false;
            return prefix[prefixIndex++];
        }
        live = true;
        if (holdOuter) {
            outer.takeRaw();
        }
        holdOuter = true;
        return outer.getCurrentToken();
    }, session };

    return runNestedParse(nested, nestedStream, table, this,
            parser::LrStop::untilComplete(stopSymbol, &live));
}

bool GnuExtensions::consumeUntilBraceEnd(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& outer,
        const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
        int stopSymbol) {
    const translation_unit::Context ctx = outer.getCurrentToken().context;
    scanner::LexicalSession& session = nested.session();
    std::size_t prefixIndex = 0;
    int depth = 0;
    bool bodyDone = false;
    bool live = false;

    parser::TokenStream nestedStream { [&]() {
        if (prefixIndex < prefixCount) {
            live = false;
            return prefix[prefixIndex++];
        }
        live = true;
        if (bodyDone) {
            return scanner::Token { scanner::Token::END, scanner::Token::END, ctx };
        }
        scanner::Token token = outer.takeRaw();
        if (token.id == "{") {
            ++depth;
        } else if (token.id == "}") {
            --depth;
            if (depth == 0) {
                bodyDone = true;
            }
        }
        return token;
    }, session };

    return runNestedParse(nested, nestedStream, table, this,
            parser::LrStop::untilLookahead(stopSymbol, scanner::Token::END, &live));
}

std::unique_ptr<Block> GnuExtensions::parseCompoundBlock(parser::TokenStream& outer,
        const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent) {
    if (outer.getCurrentToken().id != "{") {
        return nullptr;
    }
    const parser::Grammar* grammar = table.getGrammar();
    const auto compound = grammar->trySymbolId("<compound_stat>");
    if (!compound) {
        return nullptr;
    }
    const translation_unit::Context ctx = outer.getCurrentToken().context;
    const scanner::Token prefix[] = {
            { "void", "void", ctx },
            { "id", "__gnu_se", ctx },
            { "(", "(", ctx },
            { "void", "void", ctx },
            { ")", ")", ctx },
    };
    AbstractSyntaxTreeBuilder nested { grammar, parent.session(), parent.environment() };
    if (!consumeUntilBraceEnd(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *compound)) {
        return nullptr;
    }
    return nested.takeCompoundBlock();
}

std::unique_ptr<Expression> GnuExtensions::parseAssignmentExpression(parser::TokenStream& outer,
        const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent) {
    const parser::Grammar* grammar = table.getGrammar();
    const auto assignment = grammar->trySymbolId("<assignment_exp>");
    if (!assignment) {
        return nullptr;
    }
    const DummyInitPrefix prefix { outer.getCurrentToken().context };
    AbstractSyntaxTreeBuilder nested { grammar, parent.session(), parent.environment() };
    if (!consumeUntilLookahead(nested, outer, table, prefix.data(), prefix.size(), *assignment, ",")) {
        return nullptr;
    }
    return nested.takeExpression();
}

std::unique_ptr<Expression> GnuExtensions::parseCastExpression(parser::TokenStream& outer,
        const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent) {
    const parser::Grammar* grammar = table.getGrammar();
    const auto cast = grammar->trySymbolId("<cast_exp>");
    if (!cast) {
        return nullptr;
    }
    const DummyInitPrefix prefix { outer.getCurrentToken().context };
    AbstractSyntaxTreeBuilder nested { grammar, parent.session(), parent.environment() };
    if (!consumeUntilComplete(nested, outer, table, prefix.data(), prefix.size(), *cast)) {
        return nullptr;
    }
    return nested.takeExpression();
}

std::optional<TypeSpecifier> GnuExtensions::parseTypeName(parser::TokenStream& outer,
        const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent,
        const std::string& stopLookahead) {
    const parser::Grammar* grammar = table.getGrammar();
    const auto typeName = grammar->trySymbolId("<type_name>");
    if (!typeName) {
        return std::nullopt;
    }
    const translation_unit::Context ctx = outer.getCurrentToken().context;
    const scanner::Token prefix[] = {
            { "int", "int", ctx },
            { "id", "__gnu_x", ctx },
            { "=", "=", ctx },
            { "sizeof", "sizeof", ctx },
            { "(", "(", ctx },
    };
    AbstractSyntaxTreeBuilder nested { grammar, parent.session(), parent.environment() };
    if (!consumeUntilLookahead(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *typeName, stopLookahead, ")")) {
        return std::nullopt;
    }
    return nested.takeTypeSpecifier();
}

bool GnuExtensions::acceptStatementPrimary(parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable, AbstractSyntaxTreeBuilder& builder) {
    if (tokenStream.getCurrentToken().id != "(" || tokenStream.peek().id != "{") {
        return false;
    }
    const translation_unit::Context context = tokenStream.getCurrentToken().context;
    tokenStream.nextToken();
    auto block = parseCompoundBlock(tokenStream, parsingTable, builder);
    if (!block) {
        builder.err();
        return false;
    }
    if (tokenStream.getCurrentToken().id != ")") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    builder.pushExpression(std::make_unique<StatementExpression>(context, std::move(block)));
    return true;
}

bool GnuExtensions::acceptInt128(parser::TokenStream& tokenStream, AbstractSyntaxTreeBuilder& builder) {
    const scanner::Token current = tokenStream.getCurrentToken();
    const bool unsigned128 = current.lexeme == "__uint128_t";
    tokenStream.nextToken();
    tokenStream.setIdContext(parser::LexIdContext::AsIdentifier);
    if (unsigned128) {
        builder.pushTypeSpecifier(TypeSpecifier { type::unsignedInt128(), "unsigned __int128" });
    } else {
        builder.pushTypeSpecifier(TypeSpecifier { type::signedInt128(), "__int128" });
    }
    return true;
}

bool GnuExtensions::acceptRealImag(parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable, AbstractSyntaxTreeBuilder& builder) {
    const scanner::Token current = tokenStream.getCurrentToken();
    if (current.id != "id" || !isRealImagLexeme(current.lexeme)) {
        return false;
    }
    const std::string op = current.lexeme;
    tokenStream.nextToken();
    auto operand = parseCastExpression(tokenStream, parsingTable, builder);
    if (!operand) {
        builder.err();
        return false;
    }
    builder.pushExpression(std::make_unique<UnaryExpression>(
            std::make_unique<Operator>(op), std::move(operand)));
    return true;
}

bool GnuExtensions::acceptVaArg(parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable, AbstractSyntaxTreeBuilder& builder) {
    if (tokenStream.getCurrentToken().id != "id"
            || tokenStream.getCurrentToken().lexeme != "__builtin_va_arg") {
        return false;
    }
    const translation_unit::Context context = tokenStream.getCurrentToken().context;
    tokenStream.nextToken();
    if (tokenStream.getCurrentToken().id != "(") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    auto ap = parseAssignmentExpression(tokenStream, parsingTable, builder);
    if (!ap) {
        builder.err();
        return false;
    }
    if (tokenStream.getCurrentToken().id != ",") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    auto typeSpec = parseTypeName(tokenStream, parsingTable, builder);
    if (!typeSpec) {
        builder.err();
        return false;
    }
    if (tokenStream.getCurrentToken().id != ")") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    if (!typeSpec->resolveTypeofAtParseTime(builder.environment()) || !typeSpec->hasType()) {
        builder.err();
        return false;
    }
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::move(ap));
    auto call = std::make_unique<FunctionCall>(
            std::make_unique<IdentifierExpression>("__builtin_va_arg", context),
            std::move(args));
    call->setBuiltinTypeArgument(typeSpec->getType());
    builder.pushExpression(std::move(call));
    return true;
}

bool GnuExtensions::acceptTypesCompatibleP(parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable, AbstractSyntaxTreeBuilder& builder) {
    if (tokenStream.getCurrentToken().id != "id"
            || tokenStream.getCurrentToken().lexeme != "__builtin_types_compatible_p") {
        return false;
    }
    const translation_unit::Context context = tokenStream.getCurrentToken().context;
    tokenStream.nextToken();
    if (tokenStream.getCurrentToken().id != "(") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    auto type1 = parseTypeName(tokenStream, parsingTable, builder, ",");
    if (!type1) {
        builder.err();
        return false;
    }
    if (tokenStream.getCurrentToken().id != ",") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    auto type2 = parseTypeName(tokenStream, parsingTable, builder, ")");
    if (!type2) {
        builder.err();
        return false;
    }
    if (tokenStream.getCurrentToken().id != ")") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    if (!type1->resolveTypeofAtParseTime(builder.environment())
            || !type2->resolveTypeofAtParseTime(builder.environment())
            || !type1->hasType() || !type2->hasType()) {
        builder.err();
        return false;
    }
    const bool compatible = type1->getType().sameUnqualifiedType(type2->getType());
    builder.pushExpression(std::make_unique<ConstantExpression>(
            Constant { compatible ? "1" : "0", type::signedInteger(), context }));
    return true;
}

bool GnuExtensions::acceptOffsetof(parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable, AbstractSyntaxTreeBuilder& builder) {
    if (tokenStream.getCurrentToken().id != "id"
            || tokenStream.getCurrentToken().lexeme != "__builtin_offsetof") {
        return false;
    }
    const translation_unit::Context context = tokenStream.getCurrentToken().context;
    tokenStream.nextToken();
    if (tokenStream.getCurrentToken().id != "(") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    auto typeSpec = parseTypeName(tokenStream, parsingTable, builder, ",");
    if (!typeSpec) {
        builder.err();
        return false;
    }
    if (tokenStream.getCurrentToken().id != ",") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    if (tokenStream.getCurrentToken().id != "id") {
        builder.err();
        return false;
    }
    const std::string member = tokenStream.getCurrentToken().lexeme;
    tokenStream.nextToken();
    if (tokenStream.getCurrentToken().id != ")") {
        builder.err();
        return false;
    }
    tokenStream.nextToken();
    if (!typeSpec->resolveTypeofAtParseTime(builder.environment()) || !typeSpec->hasType()) {
        builder.err();
        return false;
    }
    const type::OffsetofResult off = type::resolveOffsetof(typeSpec->getType(), member);
    if (off.status == type::OffsetofStatus::Ok) {
        builder.pushExpression(std::make_unique<ConstantExpression>(
                Constant { std::to_string(off.offsetBytes), type::signedInteger(), context }));
        return true;
    }
    failOffsetof(context, off.status, member);
}

} // namespace ast
