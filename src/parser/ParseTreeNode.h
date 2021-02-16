#ifndef _PARSE_TREE_NODE_H_
#define _PARSE_TREE_NODE_H_

#include <memory>
#include <string>
#include <vector>

namespace parser {

class ParseTreeNodeVisitor;

class ParseTreeNode {
public:
	ParseTreeNode(std::string type, std::vector<std::unique_ptr<ParseTreeNode>> children);
	virtual ~ParseTreeNode() = default;

	const std::vector<std::unique_ptr<ParseTreeNode>>& getChildren() const;
	std::string getType() const;

	virtual void accept(const ParseTreeNodeVisitor& visitor) const;

private:
	std::string type;
	std::vector<std::unique_ptr<ParseTreeNode>> subtrees;
};

} // namespace parser

#endif // _PARSE_TREE_NODE_H_
