#ifndef TRANSCOMPILER_H_
#define TRANSCOMPILER_H_

#include <string>

#include "Compiler.h"

class Parser;

class TransCompiler : public Compiler {
public:
	TransCompiler(Parser& parser);
	virtual ~TransCompiler();

	void compile(TranslationUnit& translationUnit) const;

private:
	Parser& parser;
};

#endif /* TRANSCOMPILER_H_ */
