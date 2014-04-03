#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include "carrier_node.h"

/**
 * @author Vaidotas Valuckas
 * bloko mazgo klasė
 **/

class BlockNode : public CarrierNode
{
    public:
        BlockNode(string l, vector<Node *> &children);

    private:
};

#endif // _BLOCK_NODE_H_
