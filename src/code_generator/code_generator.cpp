#include "code_generator.h"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

#include "../ast/types/Type.h"
#include "LabelEntry.h"
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
    outfile << "section .text\n" << "\tglobal _start\n\n" << "___output:\n" << "\tpush eax\n" << "\tpush ebx\n" << "\tpush ecx\n" << "\tpush edx\n"
            << "\tpush ebp\n" << "\tmov ebp, esp\n" << "\tpush dword 10\n" << "\tmov eax, ecx\n" << "\tmov ecx, 4\n" << "\tmov ebx, eax\n" << "\txor edi, edi\n"
            << "\tand ebx, 0x80000000\n" << "\tjz ___loop\n" << "\tmov dword edi, 1\n" << "\tnot eax\n" << "\tadd dword eax, 1\n" << "\n___loop:\n"
            << "\tmov ebx, 10\n" << "\txor edx, edx\n" << "\tdiv ebx\n" << "\tadd edx, 0x30\n" << "\tpush edx\n" << "\tadd ecx, 4\n" << "\tcmp eax, 0\n"
            << "\tjg ___loop\n" << "\tcmp edi, 0\n" << "\tjz ___output_exit\n" << "\tadd ecx, 4\n" << "\tpush dword 45\n" << "___output_exit:\n"
            << "\tmov edx, ecx\n" << "\tmov ecx, esp\n" << "\tmov ebx, 1\n" << "\tmov eax, 4\n" << "\tint 0x80\n" << "\tmov esp, ebp\n" << "\tpop ebp\n"
            << "\tpop edx\n" << "\tpop ecx\n" << "\tpop ebx\n" << "\tpop eax\n" << "\tret\n\n";

    outfile << "___input:\n" << "\tpush eax\n" << "\tpush ebx\n" << "\tpush edx\n" << "\tpush ebp\n" << "\tmov ebp, esp\n" << "\tmov ecx, buf\n"
            << "\tmov ebx, 0\n" << "\tmov edx, 255\n" << "\tmov eax, 3\n" << "\tint 0x80\n" << "\txor eax, eax\n" << "\txor ebx, ebx\n" << "\tmov ebx, 10\n"
            << "\txor edx, edx\n" << "___to_dec:\n" << "\tcmp byte [ecx], 10\n" << "\tje ___exit_input\n" << "\tmul ebx\n" << "\tmov dl, byte [ecx]\n"
            << "\tsub dl, 48\n" << "\tadd eax, edx\n" << "\tinc ecx\n" << "\tjmp ___to_dec\n" << "___exit_input:\n" << "\tmov ecx, eax\n" << "\tmov esp, ebp\n"
            << "\tpop ebp\n" << "\tpop edx\n" << "\tpop ebx\n" << "\tpop eax\n" << "\tret\n\n";

    main = false;
    paramOffset = 0;
    eax = new Register(EAX);
    ebx = new Register(EBX);
    ecx = new Register(ECX);
    edx = new Register(EDX);
}

CodeGenerator::~CodeGenerator() {
    delete fname;
    if (outfile.is_open())
        outfile.close();
}

int CodeGenerator::generateCode(std::vector<Quadruple> code) {
    bool functionScope = false;
    for (const auto& quadruple : code) {
        auto op = quadruple.getOp();
        auto arg1 = quadruple.getArg1();
        auto arg2 = quadruple.getArg2();
        auto res = quadruple.getRes();
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
                    << "\tmov ebp, esp\n";   // ir imam esp į ebp prėjimui prie parametrų, prieš skiriant vietą lokaliems
            eax->free();
            ebx->free();
            ecx->free();
            edx->free();
            functionScope = true;
            break;
        case ENDPROC:
            break;
        case SCOPE:
            if (functionScope) {
                functionScope = false;
            }
            if (quadruple.getScopeSize()) {
                outfile << eax->free();
                outfile << ebx->free();
                outfile << ecx->free();
                outfile << edx->free();
                outfile << "\tsub esp, " << quadruple.getScopeSize() * VARIABLE_SIZE << endl;
            }
            break;
        case ENDSCOPE:
            if (functionScope && quadruple.getScopeSize()) {
                outfile << "\tadd esp, " << quadruple.getScopeSize() * VARIABLE_SIZE << endl;
            }
            break;
        case RETURN:
            ret(arg1);
            break;
        case LABEL:
            outfile << eax->free();
            outfile << ebx->free();
            outfile << ecx->free();
            outfile << edx->free();
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
            deref(arg1, res);
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

Register *CodeGenerator::getReg() {
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

Register *CodeGenerator::getReg(Register *reg) {
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

Register *CodeGenerator::getRegByName(std::string regName) const {
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

void CodeGenerator::assign(ValueEntry *arg, ValueEntry *place, std::string constant) {
    if (arg != NULL) {
        std::string regName = arg->getValue();
        Register *reg = getRegByName(regName);
        if (reg == NULL) {
            reg = getReg();
            outfile << "\tmov " << reg->getName() << ", " << getStorage(arg) << endl;
            reg->setValue(arg);
            arg->update(reg->getName());
        }
        outfile << "\tmov " << getStorage(place) << ", " << reg->getName() << endl;
        place->update("");
    } else {
        outfile << "\tmov dword " << getStorage(place) << ", " << constant << endl;
        place->update("");
    }
}

void CodeGenerator::output(ValueEntry *arg) {
    outfile << ecx->free();
    std::string place = arg->getValue();
    Register *reg = getRegByName(place);
    if (NULL != reg)
        place = reg->getName();
    else
        place = getStorage(arg);
    outfile << "\tmov ecx, " << place << endl;
    outfile << "\tcall ___output" << endl;
}

void CodeGenerator::add(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    std::string regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    Register *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
        outfile << "\tadd " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tadd " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tadd " << resReg->getName() << ", " << getStorage(arg1) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tadd " << resReg->getName() << ", " << reg2->getName() << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::sub(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    std::string regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    Register *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
        outfile << "\tsub " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tsub " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tsub " << resReg->getName() << ", " << getStorage(arg1) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tsub " << resReg->getName() << ", " << reg2->getName() << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::inc(ValueEntry *arg) {
    std::string regName = arg->getValue();
    Register *reg = getRegByName(regName);
    std::string res;
    std::string op = "\tinc ";
    if (reg != NULL)
        res = reg->getName();
    else {
        res = getStorage(arg);
        op += "dword ";
    }
    outfile << op << res << endl;
    if (reg != NULL) {
        arg->update(reg->getName());
        reg->setValue(arg);
    } else
        arg->update("");
}

void CodeGenerator::dec(ValueEntry *arg) {
    std::string regName = arg->getValue();
    Register *reg = getRegByName(regName);
    std::string res;
    std::string op = "\tdec ";
    if (reg != NULL) {
        res = reg->getName();
    } else {
        res = getStorage(arg);
        op += "dword ";
    }
    outfile << op << res << endl;
    if (reg != NULL) {
        arg->update(reg->getName());
        reg->setValue(arg);
    } else
        arg->update("");
}

void CodeGenerator::mul(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    outfile << eax->free();
    std::string regName;
    regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    if (res->getType().isPlainInteger()) {
        if (reg1 != NULL) {
            if (reg1->getName() != "eax") {
                outfile << "\tmov " << "eax, " << reg1->getName() << endl;
            }
        } else {
            outfile << "\tmov " << "eax, " << getStorage(arg1) << endl;
        }
        if (reg2 != NULL) {
            outfile << "\timul " << reg2->getName() << endl;
        } else {
            outfile << "\timul dword " << getStorage(arg2) << endl;
        }
        eax->setValue(res);
        res->update(eax->getName());
    } else {
        throw std::runtime_error { "multiplication of non integers is not implemented" };
    }
}

void CodeGenerator::div(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    outfile << edx->free();
    outfile << eax->free();
    outfile << "\txor edx, edx\n";
    std::string regName;
    regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    if (res->getType().isPlainInteger()) {
        if (reg1 != NULL) {
            if (reg1->getName() != "eax") {
                outfile << "\tmov " << eax->getName() << reg1->getName() << endl;
            }
        } else {
            outfile << "\tmov " << "eax, " << getStorage(arg1) << endl;
        }
        if (reg2 != NULL) {
            outfile << "\tidiv " << reg2->getName() << endl;
        } else {
            outfile << "\tidiv dword " << getStorage(arg2) << endl;
        }
        eax->setValue(res);
        res->update(eax->getName());
    } else {
        throw std::runtime_error { "division of non integer types is not implemented" };
    }
}

void CodeGenerator::mod(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    outfile << edx->free();
    outfile << eax->free();
    outfile << "\txor edx, edx\n";
    std::string regName;
    regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    if (res->getType().isPlainInteger()) {
        if (reg1 != NULL) {
            if (reg1->getName() != "eax") {
                outfile << "\tmov " << eax->getName() << reg1->getName() << endl;
            }
        } else {
            outfile << "\tmov " << "eax, " << getStorage(arg1) << endl;
        }
        if (reg2 != NULL) {
            outfile << "\tidiv " << reg2->getName() << endl;
        } else {
            outfile << "\tidiv dword " << getStorage(arg2) << endl;
        }
        edx->setValue(res);
        res->update(edx->getName());
    } else {
        throw std::runtime_error { "modular division of non integer types is not implemented" };
    }
}

void CodeGenerator::and_(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    std::string regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    Register *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
        outfile << "\tand " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tand " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tand " << resReg->getName() << ", " << getStorage(arg1) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tand " << resReg->getName() << ", " << reg2->getName() << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::or_(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    std::string regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    Register *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
        outfile << "\tor " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\tor " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\tor " << resReg->getName() << ", " << getStorage(arg1) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\tor " << resReg->getName() << ", " << reg2->getName() << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::xor_(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res) {
    std::string regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    regName = arg2->getValue();
    Register *reg2 = getRegByName(regName);
    Register *resReg = getReg();
    if (reg1 == NULL && reg2 == NULL) // nėra nei vienos reikšmės registre
    {
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
        outfile << "\txor " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 != NULL && reg2 == NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
        outfile << "\txor " << resReg->getName() << ", " << getStorage(arg2) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    if (reg1 == NULL && reg2 != NULL) {
        outfile << "\tmov " << resReg->getName() << ", " << reg2->getName() << endl;
        outfile << "\txor " << resReg->getName() << ", " << getStorage(arg1) << endl;
        res->update(resReg->getName());
        resReg->setValue(res);
        return;
    }
    outfile << "\tmov " << resReg->getName() << ", " << reg1->getName() << endl;
    outfile << "\txor " << resReg->getName() << ", " << reg2->getName() << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::addr(ValueEntry *arg1, ValueEntry *res) {
    outfile << store(arg1);
    Register *resReg = getReg();
    std::string baseReg = getOffsetRegister(arg1);
    unsigned offset = computeOffset(arg1->getIndex());
    outfile << "\tmov " << resReg->getName() << ", " << baseReg << endl;
    if (offset)
        outfile << "\tadd " << resReg->getName() << ", " << offset << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::deref(ValueEntry *arg1, ValueEntry *res) {
    Register *resReg = getReg();
    std::string regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    if (reg1 == NULL) {
        reg1 = getReg();
        outfile << "\tmov " << reg1->getName() << ", " << getStorage(arg1) << endl;
        //reg1->setValue(arg1);
        //arg1->update(reg1->getName());
    }
    outfile << "\tmov " << resReg->getName() << ", [" << reg1->getName() << "]\n";
    resReg->setValue(res);
    res->update(resReg->getName());
}

void CodeGenerator::deref_lval(ValueEntry *arg1, ValueEntry *res) {
    std::string regName = res->getValue();
    Register *resReg = getRegByName(regName);
    regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    if (resReg == NULL) {
        resReg = getReg();
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(res) << endl;
    }
    resReg->setValue(res);
    res->update(resReg->getName());
    if (reg1 == NULL) {
        reg1 = getReg(resReg);
        outfile << "\tmov " << reg1->getName() << ", " << getStorage(arg1) << endl;
        reg1->setValue(arg1);
        arg1->update(reg1->getName());
    }
    outfile << "\tmov [" << resReg->getName() << "], " << reg1->getName() << endl;
    //resReg->setValue(res);
    //res->update(resReg->getName());
}

void CodeGenerator::uminus(ValueEntry *arg1, ValueEntry *res) {
    std::string regName = arg1->getName();
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
        outfile << store(arg1) << endl;
        outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
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
    outfile << store(arg1) << endl;
    outfile << "\tmov " << resReg->getName() << ", " << getStorage(arg1) << endl;
    outfile << "\tnot " << resReg->getName() << endl;
    outfile << "\tadd dword " << resReg->getName() << ", 1" << endl;
    res->update(resReg->getName());
    resReg->setValue(res);
}

void CodeGenerator::shr(ValueEntry *arg1, ValueEntry *res) {
}

void CodeGenerator::shl(ValueEntry *arg1, ValueEntry *res) {
}

void CodeGenerator::input(ValueEntry *arg1) {
    outfile << ecx->free();
    outfile << "\tcall ___input\n";
    outfile << "\tmov " << getStorage(arg1) << ", " << ecx->getName() << endl;
    arg1->update(ecx->getName());
    ecx->setValue(arg1);
}

void CodeGenerator::cmp(ValueEntry *arg1, ValueEntry *arg2) {
    std::string regName;
    std::string op1 = "";
    std::string op2 = "0";
    regName = arg1->getValue();
    Register *reg1 = getRegByName(regName);
    Register *reg2 = NULL;
    if (arg2 != NULL) {
        regName = arg2->getValue();
        reg2 = getRegByName(regName);
    }
    if ((reg1 == NULL && reg1 == NULL) || arg2 == NULL) {
        reg1 = getReg();
        outfile << "\tmov " << reg1->getName() << ", " << getStorage(arg1) << endl;
        reg1->setValue(arg1);
        arg1->update(reg1->getName());
        op1 = reg1->getName();
    }
    if (reg1 != NULL)
        op1 = reg1->getName();
    if (reg2 != NULL)
        op2 = reg2->getName();
    if (op1 == "") {
        op1 = "dword ";
        op1 += getStorage(arg1);
    }
    if (reg2 == NULL && arg2 != NULL) {
        op2 = "dword ";
        op2 += getStorage(arg2);
    }

    outfile << eax->free();
    outfile << ebx->free();
    outfile << ecx->free();
    outfile << edx->free();
    outfile << "\tcmp " << op1 << ", " << op2 << endl;
}

void CodeGenerator::ret(ValueEntry *arg) {
    if (main) {
        outfile << "\tmov eax, 1\n" << "\tint 0x80\n" << "\tret\n\n";
    } else {
        std::string regName = arg->getValue();
        Register *reg = getRegByName(regName);
        if (reg != NULL && reg != eax)
            outfile << "\tmov " << eax->getName() << ", " << reg->getName() << endl;
        else
            outfile << "\tmov " << eax->getName() << ", dword " << getStorage(arg) << endl;
        outfile << "\tmov esp, ebp\n" << "\tpop ebp\n";
        outfile << "\tret\n\n";
        ebx->free();
        ecx->free();
        edx->free();
    }
}

void CodeGenerator::call(FunctionEntry *arg) {
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

void CodeGenerator::param(ValueEntry *arg) {
    unsigned offset = computeOffset(arg->getIndex()) + paramOffset;
    std::string offsetReg = getOffsetRegister(arg);

    std::string regName = arg->getValue();
    Register *reg = getRegByName(regName);
    if (reg != NULL) {
        outfile << "\tpush " << reg->getName() << "\n";
    } else {
        reg = getReg();
        outfile << "\tmov " << reg->getName() << ", ";
        if (offsetReg == "ebp")
            outfile << getStorage(arg);
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

void CodeGenerator::retrieve(ValueEntry *arg) {
    outfile << "\tmov " << getStorage(arg) << ", " << eax->getName() << std::endl;
}

std::string getOffsetRegister(ValueEntry* symbol) {
    if (symbol->isFunctionArgument()) {
        return "ebp";
    }
    return "esp";
}

std::string getStorage(ValueEntry* symbol) {
    std::ostringstream oss;
    oss << "[" << getOffsetRegister(symbol);
    if (symbol->getIndex()) {
        oss << " + " << computeOffset(symbol->getIndex());
    }
    oss << "]";
    return oss.str();
}

std::string store(ValueEntry* symbol) {
    std::string storeCommand;
    if (!symbol->isStored()) {
        storeCommand = "\tmov " + getStorage(symbol) + ", " + symbol->getValue();
    }
    return storeCommand;
}

int computeOffset(int index) {
    return index * VARIABLE_SIZE;
}

}
