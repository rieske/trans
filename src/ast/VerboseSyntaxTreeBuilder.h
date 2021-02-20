#ifndef _VERBOSE_SYNTAX_TREE_BUILDER_H_
#define _VERBOSE_SYNTAX_TREE_BUILDER_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeBuilder.h"

namespace ast {

class VerboseSyntaxTreeBuilder : public parser::SyntaxTreeBuilder {
  public:
    VerboseSyntaxTreeBuilder(std::string sourceFileName);
    virtual ~VerboseSyntaxTreeBuilder();

    void makeTerminalNode(std::string type, std::string value, const translation_unit::Context &context) override;
    void makeNonterminalNode(std::string definingSymbol, parser::Production production) override;

    std::unique_ptr<parser::SyntaxTree> build() override;

  private:
    AbstractSyntaxTreeBuilder astBuilder;
    parser::ParseTreeBuilder parseTreeBuilder;
    std::string sourceFileName;
};

} // namespace ast

#endif // _VERBOSE_SYNTAX_TREE_BUILDER_H_
