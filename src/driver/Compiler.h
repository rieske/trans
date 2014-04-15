#ifndef COMPILER_H_
#define COMPILER_H_

class TranslationUnit;

class Compiler {
public:
	virtual ~Compiler() {
	}

	virtual void compile(TranslationUnit& translationUnit) const = 0;
};

#endif /* COMPILER_H_ */
