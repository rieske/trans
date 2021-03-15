#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "Production.h"
#include "SyntaxTree.h"
#include "translation_unit/Context.h"

namespace parser {

class SyntaxTreeBuilder {
public:
    virtual ~SyntaxTreeBuilder() = default;

    virtual std::unique_ptr<SyntaxTree> build() = 0;

    virtual void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) = 0;
    virtual void makeNonterminalNode(const Production& production) = 0;
};

} // namespace parser

#endif // SYNTAXTREEBUILDER_H_
