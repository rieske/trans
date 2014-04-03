#ifndef _NODE_H_
#define _NODE_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "../quadruple.h"

using std::vector;
using std::string;
using std::ostringstream;
using std::endl;
using std::cerr;

/**
 * @author Vaidotas Valuckas
 * Abstrakti sintaksinio medžio mazgo klasė
 **/

class Node
{
    public:
        Node(string l);
        Node(string l, vector<Node *> &children);
        virtual ~Node();

        virtual string getAttr() const = 0;
        virtual ostringstream   &asXml(ostringstream &oss, unsigned depth)     const = 0;

        virtual bool getErrorFlag() const;

        vector<Quadruple *> getCode() const;
    protected:
        void printErr();
        string xmlEncode(const string &str) const;

        string          label;
        vector<Node *>  subtrees;

        bool error;
        vector<Quadruple *> code;

    private:
        void assign_label(string &l);
        void assign_children(vector<Node *> &children);
};

#endif // _NODE_H_
