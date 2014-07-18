#ifndef _PARSE_TREE_NODE_H_
#define _PARSE_TREE_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"

namespace parser {

class ParseTreeNodeVisitor;

class ParseTreeNode {
public:
	ParseTreeNode(string l);
	ParseTreeNode(string l, vector<ParseTreeNode *> &children);
	virtual ~ParseTreeNode();

	virtual string getValue() const = 0;
	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const = 0;

	std::vector<Quadruple *> getCode() const;

	std::vector<ParseTreeNode*> getChildren() const;
	std::string getLabel() const;
	virtual void accept(const ParseTreeNodeVisitor& visitor) const;

protected:
	std::string xmlEncode(const string &str) const;

	std::string label;
	std::vector<ParseTreeNode*> subtrees;

	std::vector<Quadruple *> code;

private:
	void assign_label(string &l);
	void assign_children(vector<ParseTreeNode *> children);
};

}

#endif // _PARSE_TREE_NODE_H_
