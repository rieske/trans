#include "rule.h"
#include <iostream>

using std::cerr;
using std::endl;

Rule::Rule(string *l, int ruleId)
{
    left = l;
    right = new vector<string *>;
    id = ruleId;
}

Rule::~Rule()
{
    delete right;
}

string *Rule::getLeft() const
{
    return left;
}

unsigned Rule::getId() const
{
    return id;
}

void Rule::print() const
{
    cerr << id << ": " << *left << " -> ";
    for (unsigned i = 0; i != right->size(); i++)
        cerr << *right->at(i) << " ";
    cerr << endl;
}

void Rule::printAddr() const
{
    cerr << left << " -> ";
    for (unsigned i = 0; i != right->size(); i++)
        cerr << right->at(i) << " ";
    cerr << endl;
}

void Rule::log(ostream &out) const
{
    out << id << ": " << *left << " : ";
    for (unsigned i = 0; i != right->size(); i++)
        out << *right->at(i) << " ";
    out << endl;
}

vector<string *> *Rule::getRight() const
{
    return right;
}

void Rule::addRight(string *r)
{
    right->push_back(r);
}

string Rule::rightStr() const
{
    string ret = "";
    for (vector<string *>::const_iterator it = right->begin(); it != right->end(); it++)
    {
        ret += **it;
        ret += " ";
    }    
    return ret.substr(0, ret.size()-1);
}
