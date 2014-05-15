#ifndef _ITEM_H_
#define _ITEM_H_

#include <memory>
#include <vector>


class GrammarSymbol;

using std::string;
using std::vector;
using std::ostream;

/**
 * @author Vaidotas Valuckas
 * LR(1) Item
 * [ left -> seen . expected, lookaheads ]
 **/

class Item
{
    public:
        Item(std::shared_ptr<GrammarSymbol> r);
        ~Item();

        void addSeen(std::shared_ptr<GrammarSymbol> symbol);
        void addExpected(std::shared_ptr<GrammarSymbol> expectedSymbol);
        void setExpected(vector<std::shared_ptr<GrammarSymbol>> expected);
        void addLookahead(std::shared_ptr<GrammarSymbol>);
        void mergeLookaheads(const Item& item);

        std::shared_ptr<GrammarSymbol> getLeft() const;
        vector<std::shared_ptr<GrammarSymbol>> getSeen() const;
        vector<std::shared_ptr<GrammarSymbol>> getExpected() const;
        vector<std::shared_ptr<GrammarSymbol>> getLookaheads() const;

        void print() const;

        void log(ostream &out) const;

        bool coresAreEqual(Item& rhs) const;

        bool operator==(const Item& rhs) const;
        bool operator!=(const Item& rhs) const;

    private:
        std::shared_ptr<GrammarSymbol> left;
        vector<std::shared_ptr<GrammarSymbol>> seen;
        vector<std::shared_ptr<GrammarSymbol>> expected;
        bool local_expected;
        vector<std::shared_ptr<GrammarSymbol>> lookaheads;
};

#endif // _ITEM_H_
