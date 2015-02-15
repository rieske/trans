#include "quadruple.h"

#include "../semantic_analyzer/FunctionEntry.h"
#include "../semantic_analyzer/LabelEntry.h"

namespace code_generator {

Quadruple::Quadruple(unsigned op, semantic_analyzer::ValueEntry *arg1, semantic_analyzer::ValueEntry *arg2, semantic_analyzer::ValueEntry *res) {
    this->op = op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->res = res;
}

Quadruple::Quadruple(std::string val, semantic_analyzer::ValueEntry *res) {
    this->op = ASSIGN;
    this->constant = val;
    this->res = res;
}

Quadruple::Quadruple(unsigned op, semantic_analyzer::LabelEntry *label) :
        op { op },
        label { label }
{
}

Quadruple::Quadruple(unsigned op, semantic_analyzer::FunctionEntry* function) :
        op { op },
        function { function }
{
}

Quadruple::Quadruple(unsigned op, std::map<std::string, semantic_analyzer::ValueEntry> symbols, std::map<std::string, semantic_analyzer::ValueEntry> arguments) :
        op { op },
        symbols { symbols },
        arguments { arguments }
{
}

void Quadruple::output(std::ostream &of) const {
    switch (op) {
    case ASSIGN:
        of << "\t" << res->getName() << " := " << (arg1 ? arg1->getName() : constant) << std::endl;
        break;
    case ADD:
        of << "\t" << res->getName() << " := " << arg1->getName() << " + " << arg2->getName() << std::endl;
        break;
    case SUB:
        of << "\t" << res->getName() << " := " << arg1->getName() << " - " << arg2->getName() << std::endl;
        break;
    case INC:
        of << "\t" << "INC " << arg1->getName() << std::endl;
        break;
    case DEC:
        of << "\t" << "DEC " << arg1->getName() << std::endl;
        break;
    case MUL:
        of << "\t" << res->getName() << " := " << arg1->getName() << " * " << arg2->getName() << std::endl;
        break;
    case DIV:
        of << "\t" << res->getName() << " := " << arg1->getName() << " / " << arg2->getName() << std::endl;
        break;
    case MOD:
        of << "\t" << res->getName() << " := " << arg1->getName() << " % " << arg2->getName() << std::endl;
        break;
    case AND:
        of << "\t" << res->getName() << " := " << arg1->getName() << " AND " << arg2->getName() << std::endl;
        break;
    case OR:
        of << "\t" << res->getName() << " := " << arg1->getName() << " OR " << arg2->getName() << std::endl;
        break;
    case XOR:
        of << "\t" << res->getName() << " := " << arg1->getName() << " XOR " << arg2->getName() << std::endl;
        break;
    case NOT:
        break;
    case PARAM:
        of << "\t" << "PARAM " << arg1->getName() << std::endl;
        break;
    case CALL:
        of << "\t" << "CALL " << function->getName() << ", " << function->argumentCount() << std::endl;
        break;
    case RETURN:
        of << "\t" << "RETURN " << arg1->getName() << std::endl;
        break;
    case RETRIEVE:
        of << "\t" << "RETRIEVE " << arg1->getName() << std::endl;
        break;
    case GOTO:
        of << "\t" << "GOTO " << label->getName() << std::endl;
        break;
    case ADDR:
        of << "\t" << res->getName() << " := &" << arg1->getName() << std::endl;
        break;
    case DEREF:
        of << "\t" << res->getName() << " := *" << arg1->getName() << std::endl;
        break;
    case DEREF_LVAL:
        of << "\t" << "*" << res->getName() << " := " << arg1->getName() << std::endl;
        break;
    case UMINUS:
        of << "\t" << res->getName() << " := -" << arg1->getName() << std::endl;
        break;
    case SHR:
        of << "\t" << res->getName() << " := " << arg1->getName() << " >> " << arg2->getName() << std::endl;
        break;
    case SHL:
        of << "\t" << res->getName() << " := " << arg1->getName() << " << " << arg2->getName() << std::endl;
        break;
    case CMP:
        of << "\t" << "CMP " << arg1->getName() << ", " << (arg2 ? arg2->getName() : "0") << std::endl;
        break;
    case JE:
        of << "\t" << "JE " << label->getName() << std::endl;
        break;
    case JNE:
        of << "\t" << "JNE " << label->getName() << std::endl;
        break;
    case JA:
        of << "\t" << "JA " << label->getName() << std::endl;
        break;
    case JB:
        of << "\t" << "JB " << label->getName() << std::endl;
        break;
    case JAE:
        of << "\t" << "JAE " << label->getName() << std::endl;
        break;
    case JBE:
        of << "\t" << "JBE" << label->getName() << std::endl;
        break;
    case LABEL:
        of << label->getName() << ":" << std::endl;
        break;
    case INDEX:
        of << "\t" << (res ? res->getName() : "NULL") << " := " << (arg1 ? arg1->getName() : "NULL") << "[" << (arg2 ? arg2->getName() : "NULL") << "]"
                << std::endl;
        break;
    case INDEX_ADDR:
        of << "\t" << (res ? res->getName() : "NULL") << " := " << "[" << (arg2 ? arg2->getName() : "NULL") << "]" << (arg1 ? arg1->getName() : "NULL")
                << std::endl;
        break;
    case PROC:
        of << "PROC " << function->getName() << std::endl;
        break;
    case ENDPROC:
        of << "ENDPROC " << function->getName() << std::endl;
        break;
    case IN:
        of << "\tINPUT " << arg1->getName() << std::endl;
        break;
    case OUT:
        of << "\tOUTPUT " << arg1->getName() << std::endl;
        break;
    case SCOPE:
        of << "\tBEGIN SCOPE\n";
        break;
    case ENDSCOPE:
        of << "\tEND SCOPE\n";
        break;
    default:
        std::cerr << "Unidentified quadruple! (" << op << ", " << (arg1 ? arg1->getName() : "NULL") << ", " << (arg2 ? arg2->getName() : "NULL") << ", "
                << (res ? res->getName() : "NULL") << ")\n";
    }
}

unsigned Quadruple::getOp() const {
    return op;
}

semantic_analyzer::ValueEntry *Quadruple::getArg1() const {
    return arg1;
}

semantic_analyzer::ValueEntry *Quadruple::getArg2() const {
    return arg2;
}

semantic_analyzer::ValueEntry *Quadruple::getRes() const {
    return res;
}

std::string Quadruple::getConstant() const {
    return constant;
}

semantic_analyzer::LabelEntry* Quadruple::getLabel() const {
    return label;
}

semantic_analyzer::FunctionEntry* Quadruple::getFunction() const {
    return function;
}

std::map<std::string, semantic_analyzer::ValueEntry> Quadruple::getSymbols() const {
    return symbols;
}

std::map<std::string, semantic_analyzer::ValueEntry> Quadruple::getArguments() const {
    return arguments;
}

}
