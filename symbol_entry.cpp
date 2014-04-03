#include <iostream>
#include <sstream>
#include "symbol_entry.h"

using std::cout;
using std::endl;
using std::ostringstream;

SymbolEntry::SymbolEntry(string n, string bt, string et, bool tmp, unsigned l):
name(n),
basic_type(bt),
extended_type(et),
size(4),
temp(tmp),
param(false),
line(l),
offset(0),
stored(true)
{
}

SymbolEntry::~SymbolEntry()
{
}

string SymbolEntry::getName() const
{
    return name;
}

void SymbolEntry::setBasicType(string bt)
{
    basic_type = bt;
}

string SymbolEntry::getBasicType() const
{
    return basic_type;
}

void SymbolEntry::setExtendedType(string et)
{
    extended_type = et;
}

string SymbolEntry::getExtendedType() const
{
    return extended_type;
}

bool SymbolEntry::isTemp() const
{
    return temp;
}

unsigned SymbolEntry::getLine() const
{
    return line;
}

unsigned SymbolEntry::getParamCount() const
{
    return params.size();
}

void SymbolEntry::setParam(SymbolEntry *param)
{
    params.push_back(param);
}

vector<SymbolEntry *> SymbolEntry::getParams() const
{
    return params;
}

void SymbolEntry::print() const
{
    cout << "\t" << name << "\t" << basic_type << "\t" << extended_type << "\t" 
         << (temp ? "temp" : "") << "\t" << offset 
         << "\t" << (basic_type == "label" ? "" : getStorage()) 
         << endl;
}

unsigned SymbolEntry::getOffset() const
{
    return offset;
}

void SymbolEntry::setOffset(unsigned offset)
{
    this->offset = offset;
}

string SymbolEntry::getOffsetReg() const
{
    if (param)
        return "ebp";
    else
        return "esp";
}

string SymbolEntry::getStorage() const
{
    ostringstream oss;
    if (param)
        oss << "[ebp";
    else
        oss << "[esp";
    if (offset)
        oss << " + " << offset;
    oss << "]";
    return oss.str();
}

void SymbolEntry::setParam()
{
    param = true;
}

string SymbolEntry::getValue() const
{
    if (value.size())
    {
        return value.at(0);
    }
    return "";
}

void SymbolEntry::update(string reg)
{
    if ("" != reg)
    {
        value.push_back(reg);
        stored = false;
    }
    else
    {
        value.clear();
        stored = true;
    }
}

string SymbolEntry::store()
{
    string ret = "";
    if (!stored)
    {
        ret = "\tmov ";
        ret += getStorage();
        ret += ", ";
        ret += value.at(0);
    }
    return ret;
}

unsigned SymbolEntry::getSize() const
{
    return size;
}

bool SymbolEntry::isStored() const
{
    return stored;
}

void SymbolEntry::removeReg(string reg)
{
    vector<string> newVal;
    for (unsigned i = 0; i < value.size(); i++)
        if (value.at(i) != reg)
            newVal.push_back(value.at(i));
    value = newVal;
}

bool SymbolEntry::isParam() const
{
    return param;
}
