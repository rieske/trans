#include <iostream>
#include "register.h"

using std::cerr;

Register::Register(regEnum w):
which(w),
value(NULL)
{
    switch (which)
    {
        case EAX:
            name = "eax";
            break;
        case EBX:
            name = "ebx";
            break;
        case ECX:
            name = "ecx";
            break;
        case EDX:
            name = "edx";
            break;
        default:
            cerr << "Error! No such register " << which << "!\n";
            exit(1);
    }
}

string Register::getName() const
{
    return name;
}

bool Register::isFree() const
{
    if (NULL == value)
        return true;
    return false;
}


string Register::free()
{
    string ret = "";
    if (NULL != value)
    {
        if (!value->isStored())
        {
            ret = "\tmov ";
            ret += value->getStorage();
            ret += ", ";
            ret += name;
            ret += "\n";
            value->update("");
        }
        else
            value->removeReg(name);
        value = NULL;
    }
    return ret;
}

void Register::setValue(SymbolEntry *val)
{
    value = val;
}
