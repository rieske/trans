#include "block_node.h"

BlockNode::BlockNode(string l, vector<Node *> &children):
CarrierNode(l, children)
{
    code.insert(code.begin(), new Quadruple(SCOPE, NULL, NULL, NULL));
    code.push_back(new Quadruple(ENDSCOPE, NULL, NULL, NULL));
}
