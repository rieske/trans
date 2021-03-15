#ifndef PARSETREEBUILDER_H_
#define PARSETREEBUILDER_H_

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "ParseTreeNode.h"
#include "SyntaxTreeBuilder.h"
#include "parser/Grammar.h"

namespace parser {

class ParseTreeBuilder : public SyntaxTreeBuilder {
public:
    ParseTreeBuilder(std::string sourceFileName, const Grammar* grammar);
    virtual ~ParseTreeBuilder();

    virtual std::unique_ptr<SyntaxTree> build() override;

    virtual void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) override;
    virtual void makeNonterminalNode(const parser::Production& production) override;

protected:
    std::vector<std::unique_ptr<ParseTreeNode>> getChildrenForReduction(int childrenCount);

    std::stack<std::unique_ptr<ParseTreeNode>> syntaxStack;

    std::string sourceFileName;
    const Grammar* grammar;
};

} // namespace parser

#endif // PARSETREEBUILDER_H_
