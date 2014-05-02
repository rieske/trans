#ifndef CONFIGURABLESEMANTICCOMPONENTSFACTORY_H_
#define CONFIGURABLESEMANTICCOMPONENTSFACTORY_H_

#include "SemanticComponentsFactory.h"

class ConfigurableSemanticComponentsFactory: public SemanticComponentsFactory {
public:
	ConfigurableSemanticComponentsFactory(bool usingCustomGrammar);
	virtual ~ConfigurableSemanticComponentsFactory();

	SyntaxTreeBuilder* newSyntaxTreeBuilder() const;
private:
	bool usingCustomGrammar;
};

#endif /* CONFIGURABLESEMANTICCOMPONENTSFACTORY_H_ */
