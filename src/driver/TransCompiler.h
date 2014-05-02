#ifndef TRANSCOMPILER_H_
#define TRANSCOMPILER_H_

#include <memory>

#include "Compiler.h"

class SemanticComponentsFactory;

class Parser;

class TransCompiler: public Compiler {
public:
	TransCompiler(std::unique_ptr<Parser> parser, std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory);
	virtual ~TransCompiler();

	void compile(TranslationUnit& translationUnit) const;

private:
	std::unique_ptr<Parser> parser;
	std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory;
};

#endif /* TRANSCOMPILER_H_ */
