#ifndef COMPILER_H_
#define COMPILER_H_

#include <string>

class Parser;

class Compiler {
public:
	virtual ~Compiler() {
	}

	virtual void compile(const std::string fileName) = 0;
};

#endif /* COMPILER_H_ */
