#include "Carrier.h"

#include "../code_generator/quadruple.h"

namespace semantic_analyzer {

Carrier::Carrier(string label, vector<AbstractSyntaxTreeNode*> &children) :
		NonterminalNode(label, children) {
	if (children.size() == 1) { // FIXME:
		attr = ((NonterminalNode*)children.at(0))->getValue();
	}
	for (const auto child : children) {
		vector<Quadruple *> c_code = child->getCode();
		code.insert(code.end(), c_code.begin(), c_code.end());
	}
}

}
