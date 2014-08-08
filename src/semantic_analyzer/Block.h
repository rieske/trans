#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Block: public NonterminalNode {
public:
	Block(std::unique_ptr<AbstractSyntaxTreeNode> subblock);
	Block(std::unique_ptr<AbstractSyntaxTreeNode> firstSubblock, std::unique_ptr<AbstractSyntaxTreeNode> secondSubblock);
	virtual ~Block();

	const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& getChildren() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

private:
	std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> children;
};

}

#endif // _BLOCK_NODE_H_
