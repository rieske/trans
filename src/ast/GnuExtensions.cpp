#include "GnuExtensions.h"

#include "AbstractSyntaxTreeBuilder.h"
#include "Block.h"
#include "FunctionCall.h"
#include "IdentifierExpression.h"
#include "StatementExpression.h"
#include "parser/LR1Parser.h"
#include "parser/ParsingTable.h"
#include "parser/TokenStream.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"
#include "types/Type.h"

#include <cstddef>
#include <vector>

namespace ast {

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
    if (current.id == "id" && current.lexeme == "__builtin_va_arg") {
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
    if (current.id == "id") {
        return acceptVaArg(tokenStream, parsingTable, builder);
    }
    return false;
}

bool GnuExtensions::consumeToStop(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& outer,
        const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
        int stopSymbol, const std::string& stopLookahead, bool endAfterMatchedBrace) {
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
        if (endAfterMatchedBrace && bodyDone) {
            return scanner::Token { scanner::Token::END, scanner::Token::END, ctx };
        }
        scanner::Token token = outer.takeRaw();
        if (endAfterMatchedBrace) {
            if (token.id == "{") {
                ++depth;
            } else if (token.id == "}") {
                --depth;
                if (depth == 0) {
                    bodyDone = true;
                }
            }
        }
        return token;
    }, session };

    const parser::LrStop stop { stopSymbol, stopLookahead, &live };
    if (parser::runLrParse(table, nestedStream, nested, this, stop) != parser::LrFinish::Stopped
            || nested.hasError()) {
        return false;
    }
    if (stopLookahead != scanner::Token::END) {
        outer.unget(nestedStream.getCurrentToken());
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
    AbstractSyntaxTreeBuilder nested { grammar, parent.session() };
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
    AbstractSyntaxTreeBuilder nested { grammar, parent.session() };
    if (!consumeToStop(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *assignment, ",", false)) {
        return nullptr;
    }
    return nested.takeExpression();
}

std::optional<TypeSpecifier> GnuExtensions::parseTypeName(parser::TokenStream& outer,
        const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent) {
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
    AbstractSyntaxTreeBuilder nested { grammar, parent.session() };
    if (!consumeToStop(nested, outer, table, prefix, sizeof prefix / sizeof prefix[0],
            *typeName, ")", false)) {
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

} // namespace ast
