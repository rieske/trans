#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include "scanner/Scanner.h"

#include <memory>

class CompilerComponentsFactory {
public:
	virtual ~CompilerComponentsFactory() {};

	virtual std::unique_ptr<Scanner> getScanner() const = 0;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
