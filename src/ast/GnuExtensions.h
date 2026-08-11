#ifndef AST_GNU_EXTENSIONS_H_
#define AST_GNU_EXTENSIONS_H_

#include <cstddef>
#include <memory>
#include <optional>
#include <string>

#include "parser/ParseExtensions.h"
#include "TypeSpecifier.h"

namespace parser {
class ParsingTable;
class TokenStream;
class SyntaxTreeBuilder;
}

namespace scanner {
class LexicalSession;
class Token;
}

namespace ast {

class AbstractSyntaxTreeBuilder;
class Block;
class Expression;

class GnuExtensions: public parser::ParseExtensions {
public:
    void installTypes(scanner::LexicalSession& session) const;

    std::optional<std::size_t> tryGoto(std::size_t state, parser::TokenStream& tokenStream,
            const parser::ParsingTable& parsingTable) override;
    bool accept(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
            parser::SyntaxTreeBuilder& syntaxTreeBuilder) override;

private:
    bool acceptStatementPrimary(parser::TokenStream& tokenStream,
            const parser::ParsingTable& parsingTable, AbstractSyntaxTreeBuilder& builder);
    bool acceptVaArg(parser::TokenStream& tokenStream, const parser::ParsingTable& parsingTable,
            AbstractSyntaxTreeBuilder& builder);
    bool acceptInt128(parser::TokenStream& tokenStream, AbstractSyntaxTreeBuilder& builder);

    std::unique_ptr<Block> parseCompoundBlock(parser::TokenStream& outer,
            const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent);
    std::unique_ptr<Expression> parseAssignmentExpression(parser::TokenStream& outer,
            const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent);
    std::optional<TypeSpecifier> parseTypeName(parser::TokenStream& outer,
            const parser::ParsingTable& table, AbstractSyntaxTreeBuilder& parent);

    bool consumeToStop(AbstractSyntaxTreeBuilder& nested, parser::TokenStream& outer,
            const parser::ParsingTable& table, const scanner::Token* prefix, std::size_t prefixCount,
            int stopSymbol, const std::string& stopLookahead, bool endAfterMatchedBrace);
};

} // namespace ast

#endif
