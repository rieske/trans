#ifndef AST_SYNTAX_TREE_BUILDER_H_
#define AST_SYNTAX_TREE_BUILDER_H_

#include <memory>
#include <optional>
#include <string>

#include "parser/Grammar.h"
#include "parser/Production.h"
#include "AbstractSyntaxTreeBuilderContext.h"
#include "ContextualSyntaxNodeBuilder.h"
#include "TypeSpecifier.h"
#include "scanner/LexicalSession.h"

namespace diag {
class Sink;
}

namespace parser {
class ParseExtensions;
}

namespace ast {

class AbstractSyntaxTree;
class Block;
class Expression;

class SyntaxTreeBuilder {
public:
    SyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
            std::unique_ptr<parser::ParseExtensions> extensions, bool gnuExtensions);
    SyntaxTreeBuilder(const parser::Grammar* grammar, SyntaxTreeBuilder& parent);

    static std::unique_ptr<SyntaxTreeBuilder> create(const parser::Grammar* grammar,
            scanner::LexicalSession& session, bool gnuExtensions);
    ~SyntaxTreeBuilder();

    void makeTerminalNode(std::string value, const translation_unit::Context& context);
    void makeNonterminalNode(const parser::Production& production);
    parser::ParseExtensions* parseExtensions();
    void setSink(diag::Sink* sink);
    bool hasSink() const { return sink_ != nullptr; }
    diag::Sink& sink() const;
    void err() { erred_ = true; }
    bool hasError() const { return erred_; }
    bool aborted() const;
    void fail() { treeBuilderContext.fail(); }
    bool failed() const { return treeBuilderContext.failed(); }

    std::unique_ptr<AbstractSyntaxTree> buildTree();

    scanner::LexicalSession& session();
    ParseEnvironment& environment();
    void pushExpression(std::unique_ptr<Expression> expression);
    void pushTypeSpecifier(TypeSpecifier typeSpecifier);
    std::unique_ptr<Block> popBlock();
    std::unique_ptr<Expression> takeExpression();
    std::optional<TypeSpecifier> takeTypeSpecifier();

private:
    SyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
            ParseEnvironment& parentEnvironment);
    void assertBuildable() const;

    diag::Sink* sink_ { nullptr };
    bool erred_ { false };

    ContextualSyntaxNodeBuilder syntaxNodeBuilder;
    AbstractSyntaxTreeBuilderContext treeBuilderContext;
    std::unique_ptr<parser::ParseExtensions> extensions_;
};

}

#endif
