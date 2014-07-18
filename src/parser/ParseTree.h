#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <memory>

#include "SyntaxTree.h"

namespace parser {

class ParseTree: public SyntaxTree {
public:
	ParseTree(std::unique_ptr<ParseTreeNode> top);
	virtual ~ParseTree();

	void outputXml(std::ostream& stream) const override;

private:
	std::unique_ptr<ParseTreeNode> tree;
};

} /* namespace parser */

#endif /* PARSETREE_H_ */