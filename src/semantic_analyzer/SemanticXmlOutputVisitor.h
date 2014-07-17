#ifndef SEMANTICXMLOUTPUTVISITOR_H_
#define SEMANTICXMLOUTPUTVISITOR_H_

#include "../parser/XmlOutputVisitor.h"

namespace semantic_analyzer {

class SemanticXmlOutputVisitor: public parser::XmlOutputVisitor {
public:
	SemanticXmlOutputVisitor(std::ostream* ostream);
	virtual ~SemanticXmlOutputVisitor();
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICXMLOUTPUTVISITOR_H_ */
