#include "GnuExtensions.h"

#include "AbstractSyntaxTreeBuilder.h"
#include "Block.h"
#include "Constant.h"
#include "ConstantExpression.h"
#include "FunctionCall.h"
#include "IdentifierExpression.h"
#include "StatementExpression.h"
#include "parser/Grammar.h"
#include "parser/LR1Parser.h"
#include "parser/ParsingTable.h"
#include "parser/TokenStream.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"
#include "translation_unit/Context.h"
#include "types/Type.h"
#include "util/Diagnostic.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace ast {

namespace {

// Exhaustive: new OffsetofStatus values fail to compile under -Wswitch-enum.
bool failOffsetof(const translation_unit::Context& context, type::OffsetofStatus status,
        const std::string& member, AbstractSyntaxTreeBuilder& builder) {
    switch (status) {
    case type::OffsetofStatus::Incomplete:
        builder.sink().error(context, "offsetof on incomplete type");
        break;
    case type::OffsetofStatus::Missing:
        builder.sink().error(context, "no member named ‘" + member + "’ in structure or union");
        break;
    case type::OffsetofStatus::BitField:
        builder.sink().error(context, "cannot compute offset of bit-field ‘" + member + "’");
        break;
    case type::OffsetofStatus::Ok:
        throw std::logic_error { "failOffsetof on Ok" };
    }
    builder.err();
    return false;
}

bool isInt128Lexeme(const std::string& lexeme) {
    return lexeme == "__int128" || lexeme == "__int128_t" || lexeme == "__uint128_t";
}

} // namespace

void GnuExtensions::installTypes(scanner::LexicalSession& session) const {
    session.typedefs.add("__builtin_va_list", type::builtinVaListType());
}

void GnuExtensions::cacheGrammarIds(const parser::Grammar& grammar) {
    if (cachedIds_) {
        return;
    }
    primaryExpId_ = grammar.symbolId("<primary_exp>");
    typeSpecId_ = grammar.symbolId("<type_spec>");
    unaryExpId_ = grammar.symbolId("<unary_exp>");
    cachedIds_ = true;
}

std::optional<std::size_t> GnuExtensions::tryGoto(std::size_t state, parser::TokenStream& tokenStream,
        const parser::ParsingTable& parsingTable) {
    cacheGrammarIds(*parsingTable.getGrammar());
    const scanner::Token& current = tokenStream.getCurrentToken();
    if (current.id == "(" && tokenStream.peek().id == "{") {
        return parsingTable.tryGoTo(state, primaryExpId_);
    }
    if (current.id == "id" && isInt128Lexeme(current.lexeme)) {
        return parsingTable.tryGoTo(state, typeSpecId_);
    }
    if (current.id == "id"
            && (current.lexeme == "__builtin_va_arg"
                    || current.lexeme == "__builtin_types_compatible_p"
                    || current.lexeme == "__builtin_offsetof")) {
        return parsingTable.tryGoTo(state, unaryExpId_);
    }
    return std::nullopt;
}

bool GnuExtensions::accept(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
        parser::SyntaxTreeBuilder& syntaxTreeBuilder) {
    auto& builder = static_cast<AbstractSyntaxTreeBuilder&>(syntaxTreeBuilder);
    const scanner::Token& current = tokenStream.getCurrentToken();
    if (current.id == "(") {
        return acceptStatementPrimary(tokenStream, parsingTable, builder);
    }
    if (current.id == "id" && isInt128Lexeme(current.lexeme)) {
        return acceptInt128(tokenStream, builder);
    }
    if (current.id == "id") {
        return acceptVaArg(tokenStream, parsingTable, builder)
                || acceptTypesCompatibleP(tokenStream, parsingTable, builder)
                || acceptOffsetof(tokenStream, parsingTable, builder);
    }
    return false;
}

bool GnuExtensions::isTypeExtensionToken(const scanner::Token& token) const {
    return token.id == "id" && isInt128Lexeme(token.lexeme);
}

bool GnuExtensions::consumeToStop(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& outer,
        const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
        int stopSymbol, const std::string& stopLookahead, bool endAfterMatchedBrace,
        const std::string& presentStopAs) {
    const translation_unit::Context ctx = outer.getCurrentToken().context;
    scanner::LexicalSession& session = nested.session();
    std::size_t prefixIndex = 0;
    int depth = 0;
    bool bodyDone = false;
    bool live = false;
    const std::string& innerLookahead = presentStopAs.empty() ? stopLookahead : presentStopAs;

    parser::TokenStream nestedStream { [&]() {
        if (prefixIndex < prefixCount) {
            live = false;
            return prefix[prefixIndex++];
        }
        live = true;
        if (endAfterMatchedBrace && bodyDone) {
            return scanner::Token { scanner::Token::END, scanner::Token::END, ctx };
        }
        if (!endAfterMatchedBrace) {
            scanner::Token token = outer.getCurrentToken();
            if (depth == 0 && token.id == stopLookahead) {
                if (!presentStopAs.empty()) {
                    return scanner::Token { presentStopAs, presentStopAs, token.context };
                }
                return token;
            }
            if (token.id == "(" || token.id == "[") {
                ++depth;
            } else if (token.id == ")" || token.id == "]") {
                --depth;
            }
            return outer.takeRaw();
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
    }, session, *table.getGrammar() };

    const parser::LrStop stop { stopSymbol, innerLookahead, &live };
    if (parser::runLrParse(table, nestedStream, nested, this, stop) != parser::LrFinish::Stopped
            || nested.hasError()) {
        return false;
    }
    return true;
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
    AbstractSyntaxTreeBuilder nested { grammar, parent };
    if (!consumeToStop(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *compound, scanner::Token::END, true)) {
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
    const translation_unit::Context ctx = outer.getCurrentToken().context;
    const scanner::Token prefix[] = {
            { "int", "int", ctx },
            { "id", "__gnu_x", ctx },
            { "=", "=", ctx },
    };
    AbstractSyntaxTreeBuilder nested { grammar, parent };
    if (!consumeToStop(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *assignment, ",", false)) {
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
    AbstractSyntaxTreeBuilder nested { grammar, parent };
    if (!consumeToStop(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *typeName, stopLookahead, false, ")")) {
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
    return failOffsetof(context, off.status, member, builder);
}

} // namespace ast
