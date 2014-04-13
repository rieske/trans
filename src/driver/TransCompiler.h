#ifndef TRANSCOMPILER_H_
#define TRANSCOMPILER_H_

#include <string>

#include "Compiler.h"

class TransCompiler : public Compiler {
public:
	TransCompiler(Parser& parser);
	virtual ~TransCompiler();

	void compile(const std::string fileName);

private:
	void doCompile(const std::string fileName);

	Parser& parser;
};

#endif /* TRANSCOMPILER_H_ */
