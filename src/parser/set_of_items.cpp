#include "set_of_items.h"

Set_of_items::Set_of_items()
{
    item = NULL;
    next = NULL;
}

Set_of_items::Set_of_items(Item *i)
{
    next = NULL;
    item = i;
}

Set_of_items::~Set_of_items()
{
    delete item;
    delete next;
}

void Set_of_items::print() const
{
    if (item != NULL)
        item->print();
    if (next != NULL)
        next->print();
}

void Set_of_items::log(ofstream &out) const
{
    if (item != NULL)
        item->log(out);
    if (next != NULL)
        next->log(out);
}

bool Set_of_items::addItem(Item *i)
{
    if (item == NULL)
    {
        item = i;
        return true;
    }
    else if (item->coresAreEqual(i))
    {
        item->mergeLookaheads(i);
        return false;
    }
    else if (next != NULL)
    {
        return next->addItem(i);
    }
    else
    {
        next = new Set_of_items(i);
        return true;
    }
    return false;
}

Set_of_items *Set_of_items::getNext() const
{
    return next;
}

Item *Set_of_items::getItem() const
{
    return item;
}

bool Set_of_items::operator==(const Set_of_items &rhs) const
{
    if ( (*(this->item)) != (*(rhs.item)) )
        return false;
    if ( (this->next != NULL) && (rhs.next != NULL) )
        return ( *(this->next) == *(rhs.next) );

    return true;
}
