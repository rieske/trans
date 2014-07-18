#ifndef _TERMINAL_NODE_H_
#define _TERMINAL_NODE_H_

#include "ParseTreeNode.h"
#include "scanner/Token.h"

namespace parser {

class TerminalNode: public ParseTreeNode {
public:
	TerminalNode(string type, string value);

	string getValue() const;
	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;

	void accept(const ParseTreeNodeVisitor& visitor) const;

private:
	string value;
};

}

#endif // _TERMINAL_NODE_H_
