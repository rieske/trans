#include "ParseTreeNode.h"

#include <algorithm>
#include <iterator>

ParseTreeNode::ParseTreeNode(string l) {
	error = false;
	assign_label(l);
}

ParseTreeNode::ParseTreeNode(string l, vector<ParseTreeNode *> &children) {
	error = false;
	assign_label(l);
	assign_children(children);
}

ParseTreeNode::~ParseTreeNode() {
	for (vector<ParseTreeNode *>::iterator it = subtrees.begin(); it != subtrees.end(); it++)
		delete *it;
}

void ParseTreeNode::assign_label(string &l) {
	if (l.size() > 1) {
		if (l.at(l.size() - 1) == '>')
			l = l.substr(0, l.size() - 1);
		if (l.at(0) == '<')
			l = l.substr(1, l.size() - 1);

		label = l;
	} else if (l.size() == 1) {
		label = l;
	} else {
		label = "undefined";
	}
}

void ParseTreeNode::assign_children(vector<ParseTreeNode *> children) {
	subtrees.insert(subtrees.end(), children.begin(), children.end());
}

string ParseTreeNode::xmlEncode(const string &str) const {
	string ret;
	for (unsigned i = 0; i < str.size(); i++) {
		switch (str.at(i)) {
		case '&':
			ret += "&amp;";
			continue;
		case '<':
			ret += "&lt;";
			continue;
		case '>':
			ret += "&gt;";
			continue;
		case '"':
		case '\'':
			ret += "&quot;";
			continue;
		default:
			ret += str.at(i);
		}
	}
	return ret;
}

bool ParseTreeNode::getErrorFlag() const {
	return error;
}

vector<Quadruple *> ParseTreeNode::getCode() const {
	return code;
}
