#include "SemanticXmlOutputVisitor.h"

#include <iostream>

namespace semantic_analyzer {

SemanticXmlOutputVisitor::SemanticXmlOutputVisitor(std::ostream* outputStream) :
		outputStream { outputStream } {
}

SemanticXmlOutputVisitor::~SemanticXmlOutputVisitor() {
}

void SemanticXmlOutputVisitor::visit(const AbstractSyntaxTreeNode& node) const {

}

} /* namespace semantic_analyzer */
