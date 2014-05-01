#ifndef _SET_OF_ITEMS_H_
#define _SET_OF_ITEMS_H_

#include <iostream>
#include "item.h"

using std::ostream;

/**
 * @author Vaidotas Valuckas
 * LR(1) Itemų setas
 * paprastas sąrašas su lyginimo operacija
 **/

class Set_of_items
{
    public:
        Set_of_items();
        ~Set_of_items();

        bool addItem(Item *i);

        Set_of_items   *getNext()   const;
        Item           *getItem()   const;

        void print() const;

        void log(ostream &out) const;

        bool operator==(const Set_of_items &rhs) const;

    private:
        Set_of_items(Item *i);

        Item *item;
        Set_of_items *next;
};

#endif // _SET_OF_ITEMS_H_
