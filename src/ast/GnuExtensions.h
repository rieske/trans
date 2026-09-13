#ifndef AST_GNU_EXTENSIONS_H_
#define AST_GNU_EXTENSIONS_H_

#include <cstddef>
#include <memory>
#include <optional>
#include <string>

#include "parser/ParseExtensions.h"
#include "TypeSpecifier.h"

namespace parser {
class Grammar;
class ParsingTable;
class TokenStream;
}

namespace scanner {
class LexicalSession;
class Token;
}

namespace ast {

class SyntaxTreeBuilder;
class Block;
class Expression;

class GnuExtensions: public parser::ParseExtensions {
public:
    void installTypes(scanner::LexicalSession& session) const;

    std::optional<std::size_t> tryGoto(std::size_t state, parser::TokenStream& tokenStream,
            const parser::ParsingTable& parsingTable) override;
    bool accept(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
            SyntaxTreeBuilder& syntaxTreeBuilder) override;
    bool isTypeExtensionToken(const scanner::Token& token) const override;

private:
    bool acceptStatementPrimary(parser::TokenStream& tokenStream,
            const parser::ParsingTable& parsingTable, SyntaxTreeBuilder& builder);
    bool acceptVaArg(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
            SyntaxTreeBuilder& builder);
    bool acceptTypesCompatibleP(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
            SyntaxTreeBuilder& builder);
    bool acceptOffsetof(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
            SyntaxTreeBuilder& builder);
    bool acceptInt128(parser::TokenStream& tokenStream, SyntaxTreeBuilder& builder);

    std::unique_ptr<Block> parseCompoundBlock(parser::TokenStream& outer,
            const parser::ParsingTable& table, SyntaxTreeBuilder& parent);
    std::unique_ptr<Expression> parseAssignmentExpression(parser::TokenStream& outer,
            const parser::ParsingTable& table, SyntaxTreeBuilder& parent);
    std::optional<TypeSpecifier> parseTypeName(parser::TokenStream& outer,
            const parser::ParsingTable& table, SyntaxTreeBuilder& parent,
            const std::string& stopLookahead = ")");

    bool consumeToStop(SyntaxTreeBuilder& parent, SyntaxTreeBuilder& nested,
            parser::TokenStream& outer,
            const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
            int stopSymbol, const std::string& stopLookahead, bool endAfterMatchedBrace,
            const std::string& presentStopAs = {});

    void cacheGrammarIds(const parser::Grammar& grammar);

    bool cachedIds_ { false };
    int primaryExpId_ { 0 };
    int typeSpecId_ { 0 };
    int unaryExpId_ { 0 };
    int lparenId_ { -1 };
    int idId_ { -1 };
};

} // namespace ast

#endif
