#ifndef SEMANTICCOMPONENTSFACTORY_H_
#define SEMANTICCOMPONENTSFACTORY_H_

class SyntaxTreeBuilder;

class SemanticComponentsFactory {
public:
	SemanticComponentsFactory(bool usingCustomGrammar);
	virtual ~SemanticComponentsFactory();

	SyntaxTreeBuilder* newSyntaxTreeBuilder() const;
private:
	bool usingCustomGrammar;
};

#endif /* SEMANTICCOMPONENTSFACTORY_H_ */
