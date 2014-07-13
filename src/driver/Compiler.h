#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>

class CompilerComponentsFactory;

namespace parser {
class Parser;
} /* namespace parser */

class Compiler {
public:
	Compiler(const CompilerComponentsFactory* compilerComponentsFactory);
	virtual ~Compiler();

	void compile(std::string sourceFileName) const;

private:
	std::unique_ptr<const CompilerComponentsFactory> compilerComponentsFactory;
	std::unique_ptr<parser::Parser> parser;
};

#endif /* COMPILER_H_ */
