#ifndef _CARRIER_NODE_H_
#define _CARRIER_NODE_H_

#include "nonterminal_node.h"

class CarrierNode : public NonterminalNode
{
    public:
        CarrierNode(string l, vector<Node *> &children);
        CarrierNode(string l, vector<Node *> &children, SymbolTable *scope);

        virtual string getAttr() const;
        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

    private:
        string attr;
};

#endif // _CARRIER_NODE_H_
