#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

/**
 * @author Vaidotas Valuckas
 * LR(1) Itemas
 * [ left -> seen . expected, lookaheads ]
 **/

class Item
{
    public:
        Item(string r);
        ~Item();

        void addSeen(string );
        void addExpected(string );
        void setExpected(vector<string> *e);
        void addLookahead(string);
        void mergeLookaheads(Item *i);

        string getLeft() const;
        vector<string> *getSeen() const;
        vector<string> *getExpected() const;
        vector<string> *getLookaheads() const;

        void print() const;
        void printAddr() const;

        void log(ostream &out) const;

        bool coresAreEqual(Item *) const;
        bool compare_lookaheads(const Item& rhs) const;

        bool operator==(const Item& rhs) const;
        bool operator!=(const Item& rhs) const;

    private:
        string left;
        vector<string> *seen;
        vector<string> *expected;
        bool local_expected;
        vector<string> *lookaheads;
};

#endif // _ITEM_H_
