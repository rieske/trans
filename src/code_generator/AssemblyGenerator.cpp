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
    stackMachine->unaryMinus(unaryMinus.getOperand(), unaryMinus.getResult());
}

void AssemblyGenerator::generateCodeFor(const Assign& assign) {
    stackMachine->assign(assign.getOperand(), assign.getResult());
}

void AssemblyGenerator::generateCodeFor(const AssignConstant& assignConstant) {
    stackMachine->assignConstant(assignConstant.getConstant(), assignConstant.getResult());
}

void AssemblyGenerator::generateCodeFor(const LvalueAssign& lvalueAssign) {
    stackMachine->lvalueAssign(lvalueAssign.getOperand(), lvalueAssign.getResult());
}

void AssemblyGenerator::generateCodeFor(const Argument& argument) {
    stackMachine->procedureArgument(argument.getArgumentName());
}

void code_generator::AssemblyGenerator::generateCodeFor(const Call& call) {
    stackMachine->callProcedure(call.getProcedureName());
}

void code_generator::AssemblyGenerator::generateCodeFor(const Return& returnCommand) {
    stackMachine->returnFromProcedure(returnCommand.getReturnSymbolName());
}

void AssemblyGenerator::generateCodeFor(const Retrieve& retrieve) {
    stackMachine->retrieveProcedureReturnValue(retrieve.getResultName());
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
