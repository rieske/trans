#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include <memory>

class Parser;

class Compiler;
class Scanner;

class CompilerComponentsFactory {
public:
	virtual ~CompilerComponentsFactory() {
	}

	virtual std::unique_ptr<Scanner> getScanner() const = 0;
	virtual std::unique_ptr<Compiler> getCompiler() const = 0;

protected:
	virtual std::unique_ptr<Parser> getParser() const = 0;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
