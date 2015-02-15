#include "register.h"

#include <cstdlib>
#include <iostream>

#include "code_generator.h"
#include "Value.h"

using std::cerr;

namespace code_generator {

Register::Register(regEnum w) :
        which(w), value(NULL) {
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

std::string Register::getName() const {
    return name;
}

bool Register::isFree() const {
    if (NULL == value)
        return true;
    return false;
}

std::string Register::free() {
    std::string ret = "";
    if (value) {
        if (!value->isStored()) {
            ret = "\tmov ";
            ret += getStoragePlace(value);
            ret += ", ";
            ret += name;
            ret += "\n";
            value->update("");
        } else
            value->removeReg(name);
        value = NULL;
    }
    return ret;
}

void Register::setValue(Value* val) {
    value = val;
}

}
