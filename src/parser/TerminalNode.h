#ifndef _TERMINAL_NODE_H_
#define _TERMINAL_NODE_H_

#include "ParseTreeNode.h"
#include "scanner/Token.h"

namespace parser {

class TerminalNode: public ParseTreeNode {
public:
	TerminalNode(string label, string lexeme);

	string getAttr() const;
	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;

private:
	string value;
};

}

#endif // _TERMINAL_NODE_H_
