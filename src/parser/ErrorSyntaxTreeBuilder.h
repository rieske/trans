#ifndef ERRORSYNTAXTREEBUILDER_H_
#define ERRORSYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "ParseTreeBuilder.h"

namespace parser {

class ErrorSyntaxTreeBuilder: public ParseTreeBuilder {
public:
    ErrorSyntaxTreeBuilder();
    virtual ~ErrorSyntaxTreeBuilder();

    std::unique_ptr<SyntaxTree> build() override;

    void makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) override;
    void makeNonterminalNode(const Production& ) override;
};

} // namespace parser

#endif // ERRORSYNTAXTREEBUILDER_H_
