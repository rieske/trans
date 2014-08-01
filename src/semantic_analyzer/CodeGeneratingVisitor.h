#ifndef CODEGENERATINGVISITOR_H_
#define CODEGENERATINGVISITOR_H_

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

class CodeGeneratingVisitor: public AbstractSyntaxTreeVisitor {
public:
	CodeGeneratingVisitor();
	virtual ~CodeGeneratingVisitor();
};

} /* namespace semantic_analyzer */

#endif /* CODEGENERATINGVISITOR_H_ */
