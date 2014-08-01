#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public AbstractSyntaxTreeVisitor {
public:
	SemanticAnalysisVisitor();
	virtual ~SemanticAnalysisVisitor();
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICANALYSISVISITOR_H_ */
