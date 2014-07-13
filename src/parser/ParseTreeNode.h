#ifndef _PARSE_TREE_NODE_H_
#define _PARSE_TREE_NODE_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "../code_generator/quadruple.h"

using std::vector;
using std::string;
using std::ostringstream;
using std::endl;
using std::cerr;

class ParseTreeNode {
public:
	ParseTreeNode(string l);
	ParseTreeNode(string l, vector<ParseTreeNode *> &children);
	virtual ~ParseTreeNode();

	virtual string getAttr() const = 0;
	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const = 0;

	virtual bool getErrorFlag() const;

	std::vector<Quadruple *> getCode() const;
protected:
	std::string xmlEncode(const string &str) const;

	std::string label;
	std::vector<ParseTreeNode*> subtrees;

	bool error;
	std::vector<Quadruple *> code;

private:
	void assign_label(string &l);
	void assign_children(vector<ParseTreeNode *> children);
};

#endif // _PARSE_TREE_NODE_H_
