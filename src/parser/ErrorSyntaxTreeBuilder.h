#ifndef ERRORSYNTAXTREEBUILDER_H_
#define ERRORSYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "ParseTreeBuilder.h"
#include "Production.h"

namespace parser {

class ErrorSyntaxTreeBuilder: public ParseTreeBuilder {
public:
    ErrorSyntaxTreeBuilder();
    virtual ~ErrorSyntaxTreeBuilder();

    std::unique_ptr<SyntaxTree> build() override;

    void makeTerminalNode(std::string type, std::string value, const TranslationUnitContext& context) override;
    void makeNonterminalNode(std::string, Production) override;
};

}

#endif /* ERRORSYNTAXTREEBUILDER_H_ */
