#include "register.h"

#include <cstdlib>
#include <iostream>

#include "code_generator.h"
#include "Value.h"

using std::cerr;

namespace code_generator {

Register_deprecated::Register_deprecated(regEnum w) :
        which { w },
        value { nullptr }
{
    switch (which) {
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

std::string Register_deprecated::getName() const {
    return name;
}

bool Register_deprecated::isFree() const {
    if (NULL == value)
        return true;
    return false;
}

std::string Register_deprecated::free() {
    std::string ret = "";
    if (value) {
        if (!value->isStored()) {
            ret = "\tmov ";
            ret += getMemoryAddress(value);
            ret += ", ";
            ret += name;
            ret += "\n";
            value->assignRegister("");
        } else
            value->removeReg(name);
        value = NULL;
    }
    return ret;
}

void Register_deprecated::setValue(Value* val) {
    value = val;
}

}
