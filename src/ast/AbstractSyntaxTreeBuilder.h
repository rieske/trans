#ifndef _ABSTRACT_SYNTAX_TREE_BUILDER_H_
#define _ABSTRACT_SYNTAX_TREE_BUILDER_H_

#include <memory>
#include <optional>
#include <string>

#include "parser/Grammar.h"
#include "parser/Production.h"
#include "parser/SyntaxTreeBuilder.h"
#include "AbstractSyntaxTreeBuilderContext.h"
#include "ContextualSyntaxNodeBuilder.h"
#include "TypeSpecifier.h"
#include "scanner/LexicalSession.h"

namespace parser {
class ParseExtensions;
}

namespace ast {

class Block;
class Expression;

class AbstractSyntaxTreeBuilder: public parser::SyntaxTreeBuilder {
public:
    AbstractSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
            std::unique_ptr<parser::ParseExtensions> extensions = nullptr);
    AbstractSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
            ParseEnvironment& parentEnvironment);
    virtual ~AbstractSyntaxTreeBuilder();

    void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) override;
    void makeNonterminalNode(const parser::Production& production) override;
    parser::ParseExtensions* parseExtensions() override;

    std::unique_ptr<parser::SyntaxTree> build() override;

    scanner::LexicalSession& session();
    ParseEnvironment& environment();
    void pushExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Block> takeCompoundBlock();
    std::unique_ptr<Expression> takeExpression();
    std::optional<TypeSpecifier> takeTypeSpecifier();

private:
    ContextualSyntaxNodeBuilder syntaxNodeBuilder;
    AbstractSyntaxTreeBuilderContext treeBuilderContext;
    std::unique_ptr<parser::ParseExtensions> extensions_;
};

}

#endif /* _ABSTRACT_SYNTAX_TREE_BUILDER_H_ */
