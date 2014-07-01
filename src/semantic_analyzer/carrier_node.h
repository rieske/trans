#ifndef _CARRIER_NODE_H_
#define _CARRIER_NODE_H_

#include "../parser/nonterminal_node.h"

class CarrierNode : public NonterminalNode
{
    public:
        CarrierNode(string l, vector<ParseTreeNode *> &children);
        CarrierNode(string l, vector<ParseTreeNode *> &children, SymbolTable *scope);

        virtual string getAttr() const;
        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

    private:
        string attr;
};

#endif // _CARRIER_NODE_H_
