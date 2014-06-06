#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>

#include "CompilerComponentsFactory.h"

class Parser;

class Compiler {
public:
	Compiler(const CompilerComponentsFactory* compilerComponentsFactory);
	virtual ~Compiler();

	void compile(std::string sourceFileName) const;

private:
	std::unique_ptr<const CompilerComponentsFactory> compilerComponentsFactory;
	std::unique_ptr<Parser> parser;
};

#endif /* COMPILER_H_ */
