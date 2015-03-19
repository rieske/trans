#include "AssemblyGenerator.h"

#include <string>

#include "code_generator.h"
#include "register.h"
#include "Value.h"

namespace code_generator {

AssemblyGenerator::AssemblyGenerator() {

}

void AssemblyGenerator::generateCodeFor(const StartScope& startScope) {
    if (startScope.getScopeSize() != 0) {
        //outfile << eax->free();
        //outfile << ebx->free();
        //outfile << ecx->free();
        //outfile << edx->free();
        //outfile << "\tsub esp, " << startScope.getScopeSize() * VARIABLE_SIZE << "\n";
    }
}

void AssemblyGenerator::generateCodeFor(const EndScope& endScope) {
    if (endScope.getScopeSize() != 0) {
        //outfile << "\tadd esp, " << endScope.getScopeSize() * VARIABLE_SIZE << "\n";
    }
}

void AssemblyGenerator::generateCodeFor(const Input& input) {
    Value inputValue = input.getInputValue();
    //outfile << ecx->free();
    //outfile << "\tcall ___input\n";
    //outfile << "\tmov " << getStoragePlace(inputValue) << ", " << ecx->getName() << "\n";
    //inputValue->update(ecx->getName());
    //ecx->setValue(inputValue);
}

void AssemblyGenerator::generateCodeFor(const Output& output) {
    //outfile << ecx->free();
    //std::string place = arg->getValue();
    //Register *reg = getRegByName(place);
    //if (reg)
    //    place = reg->getName();
    //else
    //    place = getStoragePlace(arg);
    //outfile << "\tmov ecx, " << place << "\n";
    //outfile << "\tcall ___output" << "\n";
}

void AssemblyGenerator::generateCodeFor(const StartProcedure& startProcedure) {
    if (startProcedure.getName() == "main") {
        //    main = true;
        //    outfile << "_start:\n";
    } else {
        //    main = false;
        //    outfile << startProcedure.getName() << ":\n";
    }
    //outfile << "\tpush ebp\n"
    //        << "\tmov ebp, esp\n";
    //eax->free();
    //ebx->free();
    //ecx->free();
    //edx->free();
}

void AssemblyGenerator::generateCodeFor(const EndProcedure& endProcedure) {
    //currentScopeValues.clear();
}

void AssemblyGenerator::generateCodeFor(const Label& label) {
    //outfile << eax->free();
    //outfile << ebx->free();
    //outfile << ecx->free();
    //outfile << edx->free();
    //outfile << label.getName() << ":\n";
}

void AssemblyGenerator::generateCodeFor(const Jump& jump) {
    switch (jump.getCondition()) {
    case JumpCondition::IF_EQUAL:
        //outfile << "\tje " << jump.getLabel() << "\n";
        break;
    case JumpCondition::IF_NOT_EQUAL:
        //outfile << "\tjne " << jump.getLabel() << "\n";
        break;
    case JumpCondition::IF_ABOVE:
        //outfile << "\tjg " << jump.getLabel() << "\n";
        break;
    case JumpCondition::IF_BELOW:
        //outfile << "\tjl " << jump.getLabel() << "\n";
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        //outfile << "\tjge " << jump.getLabel() << "\n";
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        //outfile << "\tjle " << jump.getLabel() << "\n";
        break;
    case JumpCondition::UNCONDITIONAL:
        default:
        ;
        //outfile << "\tjmp " << jump.getLabel() << "\n";
    }
}

void AssemblyGenerator::generateCodeFor(const ValueCompare& valueCompare) {
    std::string op1 = "";
    std::string op2 = "0";
    //Register *reg1 = getRegByName(arg1->getValue());
    //Register *reg2 = getRegByName(arg2->getValue());

    //if (reg1 == NULL && reg2 == NULL) {
    //    reg1 = getReg();
    //    outfile << "\tmov " << reg1->getName() << ", " << getStoragePlace(arg1) << "\n";
    //    reg1->setValue(arg1);
    //    arg1->update(reg1->getName());
    //}

    //if (reg1 != NULL) {
    //    op1 = reg1->getName();
    //} else {
    //    op1 = "dword ";
    //    op1 += getStoragePlace(arg1);
    //}

    //if (reg2 != NULL) {
    //    op2 = reg2->getName();
    //} else {
    //    op2 = "dword ";
    //    op2 += getStoragePlace(arg2);
    //}

    //outfile << "\tcmp " << op1 << ", " << op2 << "\n";
}

void AssemblyGenerator::generateCodeFor(const ZeroCompare& zeroCompare) {
    std::string op1 = "";
    //Register *reg1 = getRegByName(arg1->getValue());

    //if (reg1 == NULL) {
    //    op1 = "dword ";
    //    op1 += getStoragePlace(arg1);
    //} else {
    //    op1 = reg1.getName();
    //}

    //outfile << "\tcmp " << op1 << ", 0\n";
}

void AssemblyGenerator::generateCodeFor(const AddressOf& addressOf) {
    //store (arg1);
    //Register *resReg = getReg();
    //std::string baseReg = getOffsetRegister(arg1);
    //unsigned offset = computeOffset(arg1);
    //outfile << "\tmov " << resReg->getName() << ", " << baseReg << "\n";
    //if (offset)
    //    outfile << "\tadd " << resReg->getName() << ", " << offset << "\n";
    //res->update(resReg->getName());
    //resReg->setValue(res);
}

void AssemblyGenerator::generateCodeFor(const Dereference& dereference) {
    //Register *resReg = getReg();
    //Register *operandStorageRegister = getRegByName(operand->getValue());
    //if (operandStorageRegister == NULL) {
    //    operandStorageRegister = getReg();
    //    outfile << "\tmov " << operandStorageRegister->getName() << ", " << getMemoryAddress(operand) << "\n";
    //}
    //outfile << "\tmov " << resReg->getName() << ", [" << operandStorageRegister->getName() << "]\n";
    //resReg->setValue(result);
    //result->update(resReg->getName());

    //Register* lvalueRegister = getReg();
    //outfile << "\tmov " << lvalueRegister->getName() << ", " << getMemoryAddress(operand) << "\n";
    //lvalueRegister->setValue(lvalue);
    //lvalue->update(lvalueRegister->getName());
}

void AssemblyGenerator::generateCodeFor(const UnaryMinus& unaryMinus) {
    /*std::string regName = arg1->getName();
     Register *reg1 = getRegByName(regName);
     Register *resReg = NULL;
     if (reg1 != NULL)
     resReg = getReg(reg1);
     if (reg1 != NULL && resReg != NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tnot " << resReg->getName() << endl;
     outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 == NULL && resReg != NULL) {
     store (arg1);
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\tnot " << resReg->getName() << endl;
     outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (resReg == NULL && reg1 != NULL) {
     resReg = getReg(reg1);
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tnot " << resReg->getName() << endl;
     outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     // resReg == NULL && reg1 == NULL
     resReg = getReg();
     store (arg1);
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\tnot " << resReg->getName() << endl;
     outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
     res->update(resReg->getName());
     resReg->setValue(res);*/
}

void AssemblyGenerator::generateCodeFor(const Assign& assign) {
    /*std::string regName = operand->getValue();
     Register *operandValueRegister = getRegByName(regName);
     if (operandValueRegister == NULL) {
     operandValueRegister = getReg();
     outfile << "\tmov " << operandValueRegister->getName() << ", " << getMemoryAddress(operand) << endl;
     operandValueRegister->setValue(operand);
     operand->update(operandValueRegister->getName());
     }
     outfile << "\tmov " << getMemoryAddress(result) << ", " << operandValueRegister->getName() << endl;
     result->update("");*/
}

void AssemblyGenerator::generateCodeFor(const AssignConstant& assignConstant) {
    /*outfile << "\tmov dword " << getMemoryAddress(result) << ", " << constant << endl;
     result->update("");*/
}

void AssemblyGenerator::generateCodeFor(const LvalueAssign& lvalueAssign) {
    /*Register *resReg = getRegByName(result->getValue());
     Register *operandValueRegister = getRegByName(operand->getValue());
     if (resReg == NULL) {
     resReg = getReg();
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(result) << endl;
     resReg->setValue(result);
     result->update(resReg->getName());
     }
     if (operandValueRegister == NULL) {
     operandValueRegister = getReg(resReg);
     outfile << "\tmov " << operandValueRegister->getName() << ", " << getMemoryAddress(operand) << endl;
     operandValueRegister->setValue(operand);
     operand->update(operandValueRegister->getName());
     }
     outfile << "\tmov [" << resReg->getName() << "], " << operandValueRegister->getName() << endl;*/
}

void AssemblyGenerator::generateCodeFor(const Argument& argument) {
    // params.push_back(arg1);
}

void code_generator::AssemblyGenerator::generateCodeFor(const Call& call) {
    /*  outfile << eax->free();
     outfile << ebx->free();
     outfile << ecx->free();
     outfile << edx->free();
     for (int i = params.size() - 1; i >= 0; i--)
     param(params.at(i));
     params.clear();
     outfile << "\tcall " << call.getName() << endl;
     outfile << "\tadd esp, byte " << paramOffset << endl;
     paramOffset = 0;*/
}

void code_generator::AssemblyGenerator::generateCodeFor(const Return& returnCommand) {
    /* if (main) {
     outfile << "\tmov eax, 1\n" << "\tint 0x80\n" << "\tret\n\n";
     } else {
     std::string regName = arg->getValue();
     Register *reg = getRegByName(regName);
     if (reg != NULL && reg != eax)
     outfile << "\tmov " << eax->getName() << ", " << reg->getName() << endl;
     else
     outfile << "\tmov " << eax->getName() << ", dword " << getMemoryAddress(arg) << endl;
     outfile << "\tmov esp, ebp\n" << "\tpop ebp\n";
     outfile << "\tret\n\n";
     ebx->free();
     ecx->free();
     edx->free();
     }*/
}

void AssemblyGenerator::generateCodeFor(const Retrieve& retrieve) {
    //outfile << "\tmov " << getMemoryAddress(arg) << ", " << eax->getName() << std::endl;
}

void AssemblyGenerator::generateCodeFor(const Xor& xorCommand) {
    /*std::string regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     Register *resReg = getReg();
     if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
     {
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\txor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 != NULL && reg2 == NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\txor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 == NULL && reg2 != NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
     outfile << "\txor " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\txor " << resReg->getName() << ", " << reg2->getName() << endl;
     res->update(resReg->getName());
     resReg->setValue(res);*/
}

void AssemblyGenerator::generateCodeFor(const Or& orCommand) {
    /*std::string regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     Register *resReg = getReg();
     if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
     {
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\tor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 != NULL && reg2 == NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 == NULL && reg2 != NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
     outfile << "\tor " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tor " << resReg->getName() << ", " << reg2->getName() << endl;
     res->update(resReg->getName());
     resReg->setValue(res);*/
}

void AssemblyGenerator::generateCodeFor(const And& andCommand) {
    /*std::string regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     Register *resReg = getReg();
     if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
     {
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\tand " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 != NULL && reg2 == NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tand " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 == NULL && reg2 != NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
     outfile << "\tand " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tand " << resReg->getName() << ", " << reg2->getName() << endl;
     res->update(resReg->getName());
     resReg->setValue(res);*/
}

void AssemblyGenerator::generateCodeFor(const Add& add) {
    /*std::string regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     Register *resReg = getReg();
     if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
     {
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\tadd " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 != NULL && reg2 == NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tadd " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 == NULL && reg2 != NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
     outfile << "\tadd " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tadd " << resReg->getName() << ", " << reg2->getName() << endl;
     res->update(resReg->getName());
     resReg->setValue(res);*/
}

void AssemblyGenerator::generateCodeFor(const Sub& sub) {
    /*std::string regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     Register *resReg = getReg();
     if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
     {
     outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     outfile << "\tsub " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 != NULL && reg2 == NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tsub " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     if (reg1 == NULL && reg2 != NULL) {
     outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
     outfile << "\tsub " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
     res->update(resReg->getName());
     resReg->setValue(res);
     return;
     }
     outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
     outfile << "\tsub " << resReg->getName() << ", " << reg2->getName() << endl;
     res->update(resReg->getName());
     resReg->setValue(res);*/
}

void AssemblyGenerator::generateCodeFor(const Mul& mul) {
    /*outfile << eax->free();
     std::string regName;
     regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     if (res->getType() == Type::INTEGRAL) {
     if (reg1 != NULL) {
     if (reg1->getName() != "eax") {
     outfile << "\tmov " << "eax, " << reg1->getName() << endl;
     }
     } else {
     outfile << "\tmov " << "eax, " << getMemoryAddress(arg1) << endl;
     }
     if (reg2 != NULL) {
     outfile << "\timul " << reg2->getName() << endl;
     } else {
     outfile << "\timul dword " << getMemoryAddress(arg2) << endl;
     }
     eax->setValue(res);
     res->update(eax->getName());
     } else {
     throw std::runtime_error { "multiplication of non integers is not implemented" };
     }*/
}

void AssemblyGenerator::generateCodeFor(const Div& div) {
    /*outfile << edx->free();
     outfile << eax->free();
     outfile << "\txor edx, edx\n";
     std::string regName;
     regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     if (res->getType() == Type::INTEGRAL) {
     if (reg1 != NULL) {
     if (reg1->getName() != "eax") {
     outfile << "\tmov " << eax->getName() << reg1->getName() << endl;
     }
     } else {
     outfile << "\tmov " << "eax, " << getMemoryAddress(arg1) << endl;
     }
     if (reg2 != NULL) {
     outfile << "\tidiv " << reg2->getName() << endl;
     } else {
     outfile << "\tidiv dword " << getMemoryAddress(arg2) << endl;
     }
     eax->setValue(res);
     res->update(eax->getName());
     } else {
     throw std::runtime_error { "division of non integer types is not implemented" };
     }*/
}

void AssemblyGenerator::generateCodeFor(const Mod& mod) {
    /*outfile << edx->free();
     outfile << eax->free();
     outfile << "\txor edx, edx\n";
     std::string regName;
     regName = arg1->getValue();
     Register *reg1 = getRegByName(regName);
     regName = arg2->getValue();
     Register *reg2 = getRegByName(regName);
     if (res->getType() == Type::INTEGRAL) {
     if (reg1 != NULL) {
     if (reg1->getName() != "eax") {
     outfile << "\tmov " << eax->getName() << reg1->getName() << endl;
     }
     } else {
     outfile << "\tmov " << "eax, " << getMemoryAddress(arg1) << endl;
     }
     if (reg2 != NULL) {
     outfile << "\tidiv " << reg2->getName() << endl;
     } else {
     outfile << "\tidiv dword " << getMemoryAddress(arg2) << endl;
     }
     edx->setValue(res);
     res->update(edx->getName());
     } else {
     throw std::runtime_error { "modular division of non integer types is not implemented" };
     }*/
}

void AssemblyGenerator::generateCodeFor(const Inc& inc) {
    /*std::string regName = arg->getValue();
     Register *reg = getRegByName(regName);
     std::string res;
     std::string op = "\tinc ";
     if (reg != NULL)
     res = reg->getName();
     else {
     res = getMemoryAddress(arg);
     op += "dword ";
     }
     outfile << op << res << endl;
     if (reg != NULL) {
     arg->update(reg->getName());
     reg->setValue(arg);
     } else
     arg->update("");*/
}

void AssemblyGenerator::generateCodeFor(const Dec& dec) {
    /*std::string regName = arg->getValue();
     Register *reg = getRegByName(regName);
     std::string res;
     std::string op = "\tdec ";
     if (reg != NULL) {
     res = reg->getName();
     } else {
     res = getMemoryAddress(arg);
     op += "dword ";
     }
     outfile << op << res << endl;
     if (reg != NULL) {
     arg->update(reg->getName());
     reg->setValue(arg);
     } else
     arg->update("");*/
}

} /* namespace code_generator */
