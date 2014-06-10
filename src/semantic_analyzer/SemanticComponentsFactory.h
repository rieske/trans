#ifndef SEMANTICCOMPONENTSFACTORY_H_
#define SEMANTICCOMPONENTSFACTORY_H_
class SemanticAnalyzer;

class SemanticComponentsFactory {
public:
	SemanticComponentsFactory(bool usingCustomGrammar);
	virtual ~SemanticComponentsFactory();

	SemanticAnalyzer* newSemanticAnalyzer() const;
private:
	bool usingCustomGrammar;
};

#endif /* SEMANTICCOMPONENTSFACTORY_H_ */
