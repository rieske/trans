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
    //std::string regName = arg1->getValue();
    //Register *reg1 = getRegByName(regName);
    //if (reg1 == NULL) {
    //    reg1 = getReg();
    //    outfile << "\tmov " << reg1->getName() << ", " << getStoragePlace(arg1) << "\n";
    //}
    //outfile << "\tmov " << resReg->getName() << ", [" << reg1->getName() << "]\n";
    //resReg->setValue(res);
    //res->update(resReg->getName());
}

} /* namespace code_generator */

