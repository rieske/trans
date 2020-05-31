#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "Production.h"
#include "SyntaxTree.h"

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace parser {

class SyntaxTreeBuilder {
public:
    virtual ~SyntaxTreeBuilder() = default;

    virtual std::unique_ptr<SyntaxTree> build() = 0;

    virtual void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) = 0;
    virtual void makeNonterminalNode(std::string definingSymbol, Production production) = 0;
};

} /* namespace parser */

#endif /* SYNTAXTREEBUILDER_H_ */
