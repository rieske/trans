#include "AssemblyGenerator.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Value.h"
#include "InstructionSet.h"

namespace code_generator {

AssemblyGenerator::AssemblyGenerator(std::unique_ptr<StackMachine> stackMachine) :
        stackMachine { std::move(stackMachine) }
{
}

void AssemblyGenerator::generateAssemblyCode(std::vector<std::unique_ptr<Quadruple> > quadruples) {
    for (const auto& quadruple : quadruples) {
        quadruple->generateCode(*this);
    }
}

void AssemblyGenerator::generateCodeFor(const StartScope& startScope) {
    if (startScope.getValues().size() != 0) {
        stackMachine->storeGeneralPurposeRegisterValues();
        stackMachine->allocateStack(startScope.getValues());
    }
}

void AssemblyGenerator::generateCodeFor(const EndScope& endScope) {
    if (endScope.getScopeSize() != 0) {
        stackMachine->deallocateStack();
    }
}

void AssemblyGenerator::generateCodeFor(const Input& input) {
    stackMachine->freeIOregister();
    stackMachine->callInputProcedure();
    stackMachine->storeIOregisterIn(input.getInputSymbolName());
}

void AssemblyGenerator::generateCodeFor(const Output& output) {
    stackMachine->freeIOregister();
    stackMachine->assignIOregisterTo(output.getOutputSymbolName());
    stackMachine->callOutputProcedure();
}

void AssemblyGenerator::generateCodeFor(const StartProcedure& startProcedure) {
    stackMachine->startProcedure(startProcedure.getName());
}

void AssemblyGenerator::generateCodeFor(const EndProcedure&) {
    stackMachine->endProcedure();
}

void AssemblyGenerator::generateCodeFor(const Label& label) {
    stackMachine->label(label.getName());
}

void AssemblyGenerator::generateCodeFor(const Jump& jump) {
    stackMachine->jump(jump.getCondition(), jump.getLabel());
}

void AssemblyGenerator::generateCodeFor(const ValueCompare& valueCompare) {
    stackMachine->compare(valueCompare.getLeftSymbolName(), valueCompare.getRightSymbolName());
}

void AssemblyGenerator::generateCodeFor(const ZeroCompare& zeroCompare) {
    stackMachine->zeroCompare(zeroCompare.getSymbolName());
}

void AssemblyGenerator::generateCodeFor(const AddressOf& addressOf) {
    stackMachine->addressOf(addressOf.getOperand(), addressOf.getResult());
}

void AssemblyGenerator::generateCodeFor(const Dereference& dereference) {
    stackMachine->dereference(dereference.getOperand(), dereference.getLvalue(), dereference.getResult());
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
