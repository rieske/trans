#include "code_generator.h"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

#include "../semantic_analyzer/FunctionEntry.h"
#include "../semantic_analyzer/LabelEntry.h"
#include "../semantic_analyzer/ValueEntry.h"
#include "register.h"

using std::cerr;
using std::endl;
using std::ostringstream;

const unsigned VARIABLE_SIZE = 4;

namespace code_generator {

CodeGenerator::CodeGenerator(const char *src) {
    fname = new std::string(src);
    *fname += ".S";
    //*fname = "output/" + *fname;
    outfile.open(fname->c_str());
    if (!outfile.is_open()) {
        throw std::runtime_error { "Error creating assembler output file!" };
    }
    outfile << "section .data\n" << "\tbuf db 255\n\n";
    outfile << "section .text\n" << "\tglobal _start\n\n" << "___output:\n" << "\tpush eax\n" << "\tpush ebx\n"
            << "\tpush ecx\n" << "\tpush edx\n"
            << "\tpush ebp\n" << "\tmov ebp, esp\n" << "\tpush dword 10\n" << "\tmov eax, ecx\n" << "\tmov ecx, 4\n"
            << "\tmov ebx, eax\n" << "\txor edi, edi\n"
            << "\tand ebx, 0x80000000\n" << "\tjz ___loop\n" << "\tmov dword edi, 1\n" << "\tnot eax\n"
            << "\tadd dword eax, 1\n" << "\n___loop:\n"
            << "\tmov ebx, 10\n" << "\txor edx, edx\n" << "\tdiv ebx\n" << "\tadd edx, 0x30\n" << "\tpush edx\n"
            << "\tadd ecx, 4\n" << "\tcmp eax, 0\n"
            << "\tjg ___loop\n" << "\tcmp edi, 0\n" << "\tjz ___output_exit\n" << "\tadd ecx, 4\n"
            << "\tpush dword 45\n" << "___output_exit:\n"
            << "\tmov edx, ecx\n" << "\tmov ecx, esp\n" << "\tmov ebx, 1\n" << "\tmov eax, 4\n" << "\tint 0x80\n"
            << "\tmov esp, ebp\n" << "\tpop ebp\n"
            << "\tpop edx\n" << "\tpop ecx\n" << "\tpop ebx\n" << "\tpop eax\n" << "\tret\n\n";

    outfile << "___input:\n" << "\tpush eax\n" << "\tpush ebx\n" << "\tpush edx\n" << "\tpush ebp\n"
            << "\tmov ebp, esp\n" << "\tmov ecx, buf\n"
            << "\tmov ebx, 0\n" << "\tmov edx, 255\n" << "\tmov eax, 3\n" << "\tint 0x80\n" << "\txor eax, eax\n"
            << "\txor ebx, ebx\n" << "\tmov ebx, 10\n"
            << "\txor edx, edx\n" << "___to_dec:\n" << "\tcmp byte [ecx], 10\n" << "\tje ___exit_input\n"
            << "\tmul ebx\n" << "\tmov dl, byte [ecx]\n"
            << "\tsub dl, 48\n" << "\tadd eax, edx\n" << "\tinc ecx\n" << "\tjmp ___to_dec\n" << "___exit_input:\n"
            << "\tmov ecx, eax\n" << "\tmov esp, ebp\n"
            << "\tpop ebp\n" << "\tpop edx\n" << "\tpop ebx\n" << "\tpop eax\n" << "\tret\n\n";

    main = false;
    paramOffset = 0;
    eax = new Register_deprecated(EAX);
    ebx = new Register_deprecated(EBX);
    ecx = new Register_deprecated(ECX);
    edx = new Register_deprecated(EDX);
}

CodeGenerator::~CodeGenerator() {
    delete fname;
    if (outfile.is_open())
        outfile.close();
}

int CodeGenerator::generateCode(std::vector<Quadruple_deprecated> code) {
    for (const auto& quadruple : code) {
        auto op = quadruple.getOp();
        auto arg1 = getCurrentScopeValue(quadruple.getArg1());
        auto arg2 = getCurrentScopeValue(quadruple.getArg2());
        auto res = getCurrentScopeValue(quadruple.getRes());
        auto label = quadruple.getLabel();
        std::string constant = quadruple.getConstant();
        switch (op) {
        case PROC:
            if (quadruple.getFunction()->getName() == "main") {
                main = true;
                outfile << "_start:\n";
            } else {
                main = false;
                outfile << quadruple.getFunction()->getName() << ":\n";
            }
            outfile << "\tpush ebp\n"       // išsaugom ebp reikšmę steke
                    << "\tmov ebp, esp\n";  // ir imam esp į ebp prėjimui prie parametrų, prieš skiriant vietą lokaliems
            eax->free();
            ebx->free();
            ecx->free();
            edx->free();
            break;
        case ENDPROC:
            endScope();
            break;
        case SCOPE:
            if (!quadruple.getSymbols().empty()) {
                outfile << "\tsub esp, " << quadruple.getSymbols().size() * VARIABLE_SIZE << endl;
            }
            break;
        case ENDSCOPE:
            if (!quadruple.getSymbols().empty()) {
                outfile << "\tadd esp, " << quadruple.getSymbols().size() * VARIABLE_SIZE << endl;
            }
            break;
        case RETURN:
            ret(arg1);
            break;
        case LABEL:
            outfile << label->getName() << ":\n";
            break;
        case GOTO:
            outfile << "\tjmp " << label->getName() << endl;
            break;
        case ASSIGN:
            assign(arg1, res, constant);
            break;
        case ADD:
            add(arg1, arg2, res);
            break;
        case SUB:
            sub(arg1, arg2, res);
            break;
        case INC:
            inc(arg1);
            break;
        case DEC:
            dec(arg1);
            break;
        case MUL:
            mul(arg1, arg2, res);
            break;
        case DIV:
            div(arg1, arg2, res);
            break;
        case MOD:
            mod(arg1, arg2, res);
            break;
        case AND:
            and_(arg1, arg2, res);
            break;
        case OR:
            or_(arg1, arg2, res);
            break;
        case XOR:
            xor_(arg1, arg2, res);
            break;
        case PARAM:
            params.push_back(arg1);
            break;
        case CALL:
            call(quadruple.getFunction());
            break;
        case RETRIEVE:
            retrieve(arg1);
            break;
        case ADDR:
            addr(arg1, res);
            break;
        case DEREF:
            deref(arg1, arg2, res);
            break;
        case DEREF_LVAL:
            deref_lval(arg1, res);
            break;
        case UMINUS:
            uminus(arg1, res);
            break;
        case SHR:
            shr(arg1, res);
            break;
        case SHL:
            shl(arg1, res);
            break;
        case CMP:
            cmp(arg1, arg2);
            break;
        case JE:
            outfile << "\tje " << label->getName() << endl;
            break;
        case JNE:
            outfile << "\tjne " << label->getName() << endl;
            break;
        case JA:
            outfile << "\tjg " << label->getName() << endl;
            break;
        case JB:
            outfile << "\tjl " << label->getName() << endl;
            break;
        case JAE:
            outfile << "\tjge " << label->getName() << endl;
            break;
        case JBE:
            outfile << "\tjle " << label->getName() << endl;
            break;
        case INDEX:
            break;
        case INDEX_ADDR:
            break;
        case IN:
            input(arg1);
            break;
        case OUT:
            output(arg1);
            break;
        default:
            cerr << "Operator " << op << " is not implemented!\n";
        }
    }
    return 0;
}

int CodeGenerator::assemble() {
    if (outfile.is_open())
        outfile.close();
    std::string asmCom("nasm -O1 -f elf ");
    asmCom += *fname;
    return system(asmCom.c_str());
}

int CodeGenerator::link() {
    std::string linkCom("ld -melf_i386 -L/usr/lib32 -o ");
    linkCom += fname->substr(0, fname->size() - 2);
    linkCom += ".out ";
    linkCom += fname->substr(0, fname->size() - 2);
    linkCom += ".o";
    return system(linkCom.c_str());
}

Register_deprecated *CodeGenerator::getReg() {
    if (eax->isFree())
        return eax;
    if (ebx->isFree())
        return ebx;
    if (ecx->isFree())
        return ecx;
    if (edx->isFree())
        return edx;

    outfile << eax->free();
    return eax;
}

Register_deprecated *CodeGenerator::getReg(Register_deprecated *reg) {
    if (eax->isFree())
        return eax;
    if (ebx->isFree())
        return ebx;
    if (ecx->isFree())
        return ecx;
    if (edx->isFree())
        return edx;

    if (reg != eax) {
        outfile << eax->free();
        return eax;
    }
    if (reg != ebx) {
        outfile << ebx->free();
        return ebx;
    }
    if (reg != ecx) {
        outfile << ecx->free();
        return ecx;
    }
    if (reg != edx) {
        outfile << edx->free();
        return edx;
    }
    return NULL;
}

Register_deprecated *CodeGenerator::getRegByName(std::string regName) const {
    if (regName == "eax")
        return eax;
    if (regName == "ebx")
        return ebx;
    if (regName == "ecx")
        return ecx;
    if (regName == "edx")
        return edx;
    return NULL;
}

void CodeGenerator::output(Value *arg) {
    outfile << ecx->free();
    std::string place = arg->getAssignedRegisterName();
    Register_deprecated *reg = getRegByName(place);
    if (NULL != reg)
        place = reg->getName();
    else
        place = getMemoryAddress(arg);
    outfile << "\tmov ecx, " << place << endl;
    outfile << "\tcall ___output" << endl;
}

void CodeGenerator::add(Value *arg1, Value *arg2, Value *res) {
    std::string regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
    Register_deprecated *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        outfile << "\tadd " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tadd " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tadd " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tadd " << resReg->getName() << ", " << reg2->getName() << endl;
    res->assignRegister(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::sub(Value *arg1, Value *arg2, Value *res) {
    std::string regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
    Register_deprecated *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        outfile << "\tsub " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tsub " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tsub " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tsub " << resReg->getName() << ", " << reg2->getName() << endl;
    res->assignRegister(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::inc(Value *arg) {
    std::string regName = arg->getAssignedRegisterName();
    Register_deprecated *reg = getRegByName(regName);
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
        arg->assignRegister(reg->getName());
        reg->setValue(arg);
    } else
        arg->assignRegister("");
}

void CodeGenerator::dec(Value *arg) {
    std::string regName = arg->getAssignedRegisterName();
    Register_deprecated *reg = getRegByName(regName);
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
        arg->assignRegister(reg->getName());
        reg->setValue(arg);
    } else
        arg->assignRegister("");
}

void CodeGenerator::mul(Value *arg1, Value *arg2, Value *res) {
    outfile << eax->free();
    std::string regName;
    regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
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
        res->assignRegister(eax->getName());
    } else {
        throw std::runtime_error { "multiplication of non integers is not implemented" };
    }
}

void CodeGenerator::div(Value *arg1, Value *arg2, Value *res) {
    outfile << edx->free();
    outfile << eax->free();
    outfile << "\txor edx, edx\n";
    std::string regName;
    regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
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
        res->assignRegister(eax->getName());
    } else {
        throw std::runtime_error { "division of non integer types is not implemented" };
    }
}

void CodeGenerator::mod(Value *arg1, Value *arg2, Value *res) {
    outfile << edx->free();
    outfile << eax->free();
    outfile << "\txor edx, edx\n";
    std::string regName;
    regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
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
        res->assignRegister(edx->getName());
    } else {
        throw std::runtime_error { "modular division of non integer types is not implemented" };
    }
}

void CodeGenerator::and_(Value *arg1, Value *arg2, Value *res) {
    std::string regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
    Register_deprecated *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        outfile << "\tand " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tand " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tand " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tand " << resReg->getName() << ", " << reg2->getName() << endl;
    res->assignRegister(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::or_(Value *arg1, Value *arg2, Value *res) {
    std::string regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
    Register_deprecated *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        outfile << "\tor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tor " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tor " << resReg->getName() << ", " << reg2->getName() << endl;
    res->assignRegister(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::xor_(Value *arg1, Value *arg2, Value *res) {
    std::string regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    regName = arg2->getAssignedRegisterName();
    Register_deprecated *reg2 = getRegByName(regName);
    Register_deprecated *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        outfile << "\txor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\txor " << resReg->getName() << ", " << getMemoryAddress(arg2) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\txor " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\txor " << resReg->getName() << ", " << reg2->getName() << endl;
    res->assignRegister(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::addr(Value *arg1, Value *res) {
    store(arg1);
    Register_deprecated *resReg = getReg();
    std::string baseReg = getOffsetRegister(arg1);
    unsigned offset = computeOffset(arg1);
    outfile << "\tmov " << resReg->getName() << ", " << baseReg << endl;
    if (offset)
        outfile << "\tadd " << resReg->getName() << ", " << offset << endl;
    res->assignRegister(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::deref(Value *operand, Value *lvalue, Value *result) {
    Register_deprecated *resReg = getReg();
    Register_deprecated *operandStorageRegister = getRegByName(operand->getAssignedRegisterName());
    if (operandStorageRegister == NULL) {
        operandStorageRegister = getReg();
        outfile << "\tmov " << operandStorageRegister->getName() << ", " << getMemoryAddress(operand) << endl;
    }
    outfile << "\tmov " << resReg->getName() << ", [" << operandStorageRegister->getName() << "]\n";
    resReg->setValue(result);
    result->assignRegister(resReg->getName());

    Register_deprecated* lvalueRegister = getReg();
    outfile << "\tmov " << lvalueRegister->getName() << ", " << getMemoryAddress(operand) << endl;
    lvalueRegister->setValue(lvalue);
    lvalue->assignRegister(lvalueRegister->getName());
}

void CodeGenerator::deref_lval(Value *operand, Value *lvalue) {
    Register_deprecated *resReg = getRegByName(lvalue->getAssignedRegisterName());
    Register_deprecated *operandValueRegister = getRegByName(operand->getAssignedRegisterName());
    if (resReg == NULL) {
        resReg = getReg();
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(lvalue) << endl;
        resReg->setValue(lvalue);
        lvalue->assignRegister(resReg->getName());
    }
    if (operandValueRegister == NULL) {
        operandValueRegister = getReg(resReg);
        outfile << "\tmov " << operandValueRegister->getName() << ", " << getMemoryAddress(operand) << endl;
        operandValueRegister->setValue(operand);
        operand->assignRegister(operandValueRegister->getName());
    }
    outfile << "\tmov [" << resReg->getName() << "], " << operandValueRegister->getName() << endl;
}

void CodeGenerator::assign(Value *operand, Value *result, std::string constant) {
    if (operand != NULL) {
        std::string regName = operand->getAssignedRegisterName();
        Register_deprecated *operandValueRegister = getRegByName(regName);
        if (operandValueRegister == NULL) {
            operandValueRegister = getReg();
            outfile << "\tmov " << operandValueRegister->getName() << ", " << getMemoryAddress(operand) << endl;
            operandValueRegister->setValue(operand);
            operand->assignRegister(operandValueRegister->getName());
        }
        outfile << "\tmov " << getMemoryAddress(result) << ", " << operandValueRegister->getName() << endl;
        result->assignRegister("");
    } else {
        outfile << "\tmov dword " << getMemoryAddress(result) << ", " << constant << endl;
        result->assignRegister("");
    }
}

void CodeGenerator::uminus(Value *arg1, Value *res) {
    Register_deprecated *reg1 = getRegByName(arg1->getAssignedRegisterName());
    if (!reg1) {
        Register_deprecated * resReg = getReg();
        outfile << "\tmov " << resReg->getName() << ", " << getMemoryAddress(arg1) << endl;
        outfile << "\tnot " << resReg->getName() << endl;
        outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
    } else {
        Register_deprecated *resReg = getReg(reg1);
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tnot " << resReg->getName() << endl;
        outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
        res->assignRegister(resReg->getName());
        resReg->setValue(res);
    }
}

void CodeGenerator::shr(Value *arg1, Value *res) {
}

void CodeGenerator::shl(Value *arg1, Value *res) {
}

void CodeGenerator::input(Value *arg1) {
    outfile << ecx->free();
    outfile << "\tcall ___input\n";
    outfile << "\tmov " << getMemoryAddress(arg1) << ", " << ecx->getName() << endl;
    arg1->assignRegister(ecx->getName());
    ecx->setValue(arg1);
}

void CodeGenerator::cmp(Value *arg1, Value *arg2) {
    // dafuq?
    std::string regName;
    std::string op1 = "";
    std::string op2 = "0";
    regName = arg1->getAssignedRegisterName();
    Register_deprecated *reg1 = getRegByName(regName);
    Register_deprecated *reg2 = NULL;
    if (arg2 != NULL) {
        regName = arg2->getAssignedRegisterName();
        reg2 = getRegByName(regName);
    }
    if ((reg1 == NULL && reg2 == NULL) || arg2 == NULL) {
        reg1 = getReg();
        outfile << "\tmov " << reg1->getName() << ", " << getMemoryAddress(arg1) << endl;
        reg1->setValue(arg1);
        arg1->assignRegister(reg1->getName());
        op1 = reg1->getName();
    }

    if (reg1 != NULL)
        op1 = reg1->getName();
    if (op1 == "") {
        op1 = "dword ";
        op1 += getMemoryAddress(arg1);
    }

    if (reg2 != NULL)
        op2 = reg2->getName();
    if (reg2 == NULL && arg2 != NULL) {
        op2 = "dword ";
        op2 += getMemoryAddress(arg2);
    }

    outfile << "\tcmp " << op1 << ", " << op2 << endl;
}

void CodeGenerator::ret(Value *arg) {
    if (main) {
        outfile << "\tmov eax, 1\n" << "\tint 0x80\n" << "\tret\n\n";
    } else {
        std::string regName = arg->getAssignedRegisterName();
        Register_deprecated *reg = getRegByName(regName);
        if (reg != NULL && reg != eax)
            outfile << "\tmov " << eax->getName() << ", " << reg->getName() << endl;
        else
            outfile << "\tmov " << eax->getName() << ", dword " << getMemoryAddress(arg) << endl;
        outfile << "\tmov esp, ebp\n" << "\tpop ebp\n";
        outfile << "\tret\n\n";
        ebx->free();
        ecx->free();
        edx->free();
    }
}

void CodeGenerator::call(semantic_analyzer::FunctionEntry *arg) {
    outfile << eax->free();
    outfile << ebx->free();
    outfile << ecx->free();
    outfile << edx->free();
    for (int i = params.size() - 1; i >= 0; i--)
        param(params.at(i));
    params.clear();
    outfile << "\tcall " << arg->getName() << endl;
    outfile << "\tadd esp, byte " << paramOffset << endl;
    paramOffset = 0;
}

void CodeGenerator::param(Value *arg) {
    unsigned offset = computeOffset(arg) + paramOffset;
    std::string offsetReg = getOffsetRegister(arg);

    std::string regName = arg->getAssignedRegisterName();
    Register_deprecated *reg = getRegByName(regName);
    if (reg != NULL) {
        outfile << "\tpush " << reg->getName() << "\n";
    } else {
        reg = getReg();
        outfile << "\tmov " << reg->getName() << ", ";
        if (offsetReg == "ebp")
            outfile << getMemoryAddress(arg);
        else {
            outfile << "[" << offsetReg;
            if (offset)
                outfile << " + " << offset;
            outfile << "]";
        }
        outfile << "\n";
        outfile << "\tpush " << reg->getName() << endl;
    }
    paramOffset += VARIABLE_SIZE;
}

void CodeGenerator::retrieve(Value *arg) {
    outfile << "\tmov " << getMemoryAddress(arg) << ", " << eax->getName() << std::endl;
}

void CodeGenerator::store(Value* symbol) {
    if (!symbol->isStored()) {
        outfile << "\tmov " << getMemoryAddress(symbol) << ", " << symbol->getAssignedRegisterName() << "\n";
    }
}

std::string getOffsetRegister(Value* symbol) {
    if (symbol->isFunctionArgument()) {
        return "ebp";
    }
    return "esp";
}

std::string getMemoryAddress(Value* symbol) {
    std::ostringstream oss;
    oss << "[" << getOffsetRegister(symbol);
    int offset = computeOffset(symbol);
    if (offset) {
        oss << " + " << offset;
    }
    oss << "]";
    return oss.str();
}

int computeOffset(Value* symbol) {
    if (symbol->isFunctionArgument()) {
        return (symbol->getIndex() + 2) * VARIABLE_SIZE;
    }
    return symbol->getIndex() * VARIABLE_SIZE;
}

Value* CodeGenerator::getCurrentScopeValue(semantic_analyzer::ValueEntry* optionalValue) {
    if (!optionalValue) {
        return nullptr;
    }
    if (currentScopeValues.end() == currentScopeValues.find(optionalValue->getName())) {
        currentScopeValues.insert(std::make_pair<std::string, Value>(optionalValue->getName(),
                // FIXME:
                { optionalValue->getName(), optionalValue->getIndex(), Type::INTEGRAL,
                        optionalValue->isFunctionArgument() }));
    }
    return &currentScopeValues.at(optionalValue->getName());
}

void CodeGenerator::endScope() {
    eax->free();
    ebx->free();
    ecx->free();
    edx->free();
    currentScopeValues.clear();
}

}

