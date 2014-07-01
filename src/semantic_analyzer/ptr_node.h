#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../parser/nonterminal_node.h"

class PtrNode : public NonterminalNode
{
    public:
        PtrNode(string l, vector<ParseTreeNode *> &children, Production production);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        string getType() const;

    private:
        string type;
};

#endif // _PTR_NODE_H_
