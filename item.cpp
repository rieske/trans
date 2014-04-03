#include "item.h"
#include <iostream>
#include <stdexcept>

using std::cerr;
using std::endl;

Item::Item(string *str)
{
    left = str;
    seen = new vector<string *>;
    expected = new vector<string *>;
    local_expected = true;
    lookaheads = new vector<string *>;
}

Item::~Item()
{
    delete seen;
    if (local_expected)
        delete expected;
    delete lookaheads;
}

bool Item::operator==(const Item& rhs) const
{
    if (*this->left != *rhs.left)
        return false;
    if (*this->seen != *rhs.seen)
        return false;
    if (*this->expected != *rhs.expected)
        return false;
    if (this->lookaheads->size() != rhs.lookaheads->size())
        return false;
    if (!compare_lookaheads(rhs))
        return false;
    return true;
}

bool Item::compare_lookaheads(const Item& rhs) const
{
    unsigned from = 0;
    for (vector<string *>::const_iterator it1 = lookaheads->begin(); it1 != lookaheads->end(); it1++)
    {
        bool contains = false;
        for (vector<string *>::const_iterator it2 = rhs.lookaheads->begin() + from; it2 != rhs.lookaheads->end(); it2++)
        {
            if (*it1 == *it2)
            {
                contains = true;
                from++;
                break;
            }
        }
        if (!contains)
            return false;
    }
    return true;
}

bool Item::operator!=(const Item& rhs) const
{
    return !(*this == rhs);
}

void Item::print() const
{
    cerr << "[ " << *left << " -> ";
    for (unsigned i = 0; i < seen->size(); i++)
        cerr << *seen->at(i) << " ";
    cerr << ". ";
    for (unsigned i = 0; i < expected->size(); i++)
        cerr << *expected->at(i) << " ";
    cerr << ", ";
    for (unsigned i = 0; i < lookaheads->size(); i++)
    {
        cerr << *lookaheads->at(i);
        if (i != lookaheads->size()-1)
            cerr << "/";
    }
    cerr << " ]" << endl;
}

void Item::printAddr() const
{
    cerr << "[ " << left << " -> ";
    for (unsigned i = 0; i < seen->size(); i++)
        cerr << seen->at(i) << " ";
    cerr << ". ";
    for (unsigned i = 0; i < expected->size(); i++)
        cerr << expected->at(i) << " ";
    cerr << ", ";
    for (unsigned i = 0; i < lookaheads->size(); i++)
    {
        cerr << lookaheads->at(i);
        if (i != lookaheads->size()-1)
            cerr << "/";
    }
    cerr << " ]" << endl;
}

void Item::log(ofstream &out) const
{
    out << "[ " << *left << " -> ";
    for (unsigned i = 0; i < seen->size(); i++)
        out << *seen->at(i) << " ";
    out << ". ";
    for (unsigned i = 0; i < expected->size(); i++)
        out << *expected->at(i) << " ";
    out << ", ";
    for (unsigned i = 0; i < lookaheads->size(); i++)
    {
        out << *lookaheads->at(i);
        if (i != lookaheads->size()-1)
            out << "/";
    }
    out << " ]" << endl;
}

bool Item::coresAreEqual(Item *rhs) const
{
    if (*this->left != *rhs->left)
        return false;
    if (*this->seen != *rhs->seen)
        return false;
    if (*this->expected != *rhs->expected)
        return false;
    return true;
}

void Item::mergeLookaheads(Item *it)
{
    if (lookaheads->empty())
    {
        for (unsigned i = 0; i < it->lookaheads->size(); i++)
            lookaheads->push_back(it->lookaheads->at(i));
        return;
    }
    for (unsigned i = 0; i < it->lookaheads->size(); i++)
    {
        bool insert = true;
        unsigned j = 0;
        for (; j < this->lookaheads->size(); j++)
        {
            if (it->lookaheads->at(i) == this->lookaheads->at(j))
            {
                insert = false;
                break;
            }
        }
        if (insert)
            this->lookaheads->push_back(it->lookaheads->at(i));
    }
}

void Item::addSeen(string *str)
{
    seen->push_back(str);
}

void Item::addExpected(string *str)
{
    expected->push_back(str);
}

void Item::setExpected(vector<string *> *e)
{
    expected = e;
    local_expected = false;
}

void Item::addLookahead(string *lookahead)
{
    lookaheads->push_back(lookahead);
}

string *Item::getLeft() const
{
    return left;
}

vector<string *> *Item::getSeen() const
{
    return seen;
}

vector<string *> *Item::getExpected() const
{
    return expected;
}

vector<string *> *Item::getLookaheads() const
{
    return lookaheads;
}

