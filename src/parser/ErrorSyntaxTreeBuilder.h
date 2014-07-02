#ifndef ERRORSYNTAXTREEBUILDER_H_
#define ERRORSYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "../parser/ParseTreeBuilder.h"

class ErrorSyntaxTreeBuilder: public ParseTreeBuilder {
public:
	ErrorSyntaxTreeBuilder();
	virtual ~ErrorSyntaxTreeBuilder();

	std::unique_ptr<SyntaxTree> build() override;

	void makeTerminalNode(const Token&) override;
	void makeNonterminalNode(std::string, Production) override;
};

#endif /* ERRORSYNTAXTREEBUILDER_H_ */
