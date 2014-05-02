#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <string>

class SyntaxTreeBuilder {
public:
	virtual ~SyntaxTreeBuilder();

	virtual void withSourceFileName(std::string fileName);

protected:
	std::string sourceFileName;
};

#endif /* SYNTAXTREEBUILDER_H_ */
