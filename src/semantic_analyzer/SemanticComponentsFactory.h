#ifndef SEMANTICCOMPONENTSFACTORY_H_
#define SEMANTICCOMPONENTSFACTORY_H_

class SyntaxTreeBuilder;

class SemanticComponentsFactory {
public:
	virtual ~SemanticComponentsFactory() {};

	virtual SyntaxTreeBuilder* newSyntaxTreeBuilder() const = 0;
};

#endif /* SEMANTICCOMPONENTSFACTORY_H_ */
