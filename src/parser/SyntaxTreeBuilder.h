#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <string>

#include "Production.h"
#include "translation_unit/Context.h"

namespace diag {
class Sink;
}

namespace parser {

class ParseExtensions;

class SyntaxTreeBuilder {
public:
    virtual ~SyntaxTreeBuilder();

    virtual void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) = 0;
    virtual void makeNonterminalNode(const Production& production) = 0;

    virtual ParseExtensions* parseExtensions() { return nullptr; }

    virtual void setSink(diag::Sink* sink) { sink_ = sink; }
    bool hasSink() const { return sink_ != nullptr; }
    diag::Sink& sink() const;

    void err();
    bool hasError() const { return erred; }
    // Syntax err() or a CSNB diagnostic. CSNB does not set erred (no syntax footer).
    virtual bool aborted() const { return hasError(); }

protected:
    void assertBuildable() const;

private:
    diag::Sink* sink_ { nullptr };
    bool erred {false};
};

} // namespace parser

#endif // SYNTAXTREEBUILDER_H_
