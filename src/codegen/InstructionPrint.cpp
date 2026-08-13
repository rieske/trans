#include "Instruction.h"

#include <sstream>
#include <stdexcept>

namespace codegen {

namespace {

void printJump(std::ostream& stream, const Instruction& instruction) {
    stream << "\t";
    switch (instruction.cond) {
    case JumpCondition::IF_EQUAL:
        stream << "JE ";
        break;
    case JumpCondition::IF_NOT_EQUAL:
        stream << "JNE ";
        break;
    case JumpCondition::IF_ABOVE:
        stream << "JG ";
        break;
    case JumpCondition::IF_BELOW:
        stream << "JL ";
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        stream << "JGE ";
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        stream << "JLE ";
        break;
    case JumpCondition::IF_ABOVE_U:
        stream << "JA ";
        break;
    case JumpCondition::IF_BELOW_U:
        stream << "JB ";
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL_U:
        stream << "JAE ";
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL_U:
        stream << "JBE ";
        break;
    case JumpCondition::UNCONDITIONAL:
        stream << "GOTO ";
        break;
    default:
        throw std::logic_error { "printJump: unhandled JumpCondition" };
    }
    stream << instruction.arg0 << "\n";
}

} // namespace

void print(std::ostream& stream, const Instruction& instruction) {
    switch (instruction.op) {
    case Op::Add:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " + " << instruction.arg1 << "\n";
        return;
    case Op::Sub:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " - " << instruction.arg1 << "\n";
        return;
    case Op::Mul:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " * " << instruction.arg1 << "\n";
        return;
    case Op::Div:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " / " << instruction.arg1
               << (instruction.unsignedArith ? " (u)\n" : "\n");
        return;
    case Op::Mod:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " % " << instruction.arg1
               << (instruction.unsignedArith ? " (u)\n" : "\n");
        return;
    case Op::And:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " AND " << instruction.arg1 << "\n";
        return;
    case Op::Or:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " OR " << instruction.arg1 << "\n";
        return;
    case Op::Xor:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " XOR " << instruction.arg1 << "\n";
        return;
    case Op::Shl:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " << " << instruction.arg1 << "\n";
        return;
    case Op::Shr:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << " >> " << instruction.arg1
               << (instruction.logicalShift ? " (logical)\n" : "\n");
        return;
    case Op::UnaryMinus:
        stream << "\t" << instruction.result << " := -" << instruction.arg0 << "\n";
        return;
    case Op::UnaryNot:
        stream << "\t" << instruction.result << " := ~" << instruction.arg0 << "\n";
        return;
    case Op::Inc:
        if (instruction.imm == 1) {
            stream << "\tINC " << instruction.arg0 << "\n";
        } else {
            stream << "\t" << instruction.arg0 << " := " << instruction.arg0 << " + " << instruction.imm << "\n";
        }
        return;
    case Op::Dec:
        if (instruction.imm == 1) {
            stream << "\tDEC " << instruction.arg0 << "\n";
        } else {
            stream << "\t" << instruction.arg0 << " := " << instruction.arg0 << " - " << instruction.imm << "\n";
        }
        return;
    case Op::Assign:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << "\n";
        return;
    case Op::Widen:
        stream << "\t" << instruction.result << " := widen " << instruction.arg0 << "\n";
        return;
    case Op::AssignConstant:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << "\n";
        return;
    case Op::AssignLabelAddress:
        stream << "\t" << instruction.result << " := &" << instruction.arg0 << "\n";
        return;
    case Op::LvalueAssign:
        stream << "\t*" << instruction.result << " := " << instruction.arg0
               << " [" << instruction.accessSizeBytes << "]\n";
        return;
    case Op::AddressOf:
        stream << "\t" << instruction.result << " := &" << instruction.arg0 << "\n";
        return;
    case Op::Dereference:
        stream << "\t" << instruction.result << " := *" << instruction.arg0
               << " (lvalue " << instruction.arg1 << ")\n";
        return;
    case Op::IndexAddress:
        stream << "\t" << instruction.result << " := &" << instruction.arg0 << "[" << instruction.arg1
               << "] *" << instruction.imm << "\n";
        return;
    case Op::CopyPart:
        stream << "\t" << instruction.result << " := " << instruction.arg0 << "[+" << instruction.imm << "]\n";
        return;
    case Op::FieldAddress:
        stream << "\t" << instruction.result << " := &" << instruction.arg0 << "+" << instruction.imm << "\n";
        return;
    case Op::FunctionAddress:
        stream << "\t" << instruction.result << " := &" << instruction.arg0 << "\n";
        return;
    case Op::ValueCompare:
        stream << "\tCMP " << instruction.arg0 << ", " << instruction.arg1 << "\n";
        return;
    case Op::ZeroCompare:
        stream << "\tCMP " << instruction.arg0 << ", 0\n";
        return;
    case Op::Jump:
        printJump(stream, instruction);
        return;
    case Op::Label:
        stream << instruction.arg0 << ":\n";
        return;
    case Op::Argument:
        stream << "\tPARAM " << instruction.arg0 << "\n";
        return;
    case Op::Call:
        if (instruction.callIndirect) {
            stream << "\tCALL *" << instruction.arg0;
        } else {
            stream << "\tCALL " << instruction.arg0;
        }
        if (!instruction.memoryReturnDest.empty()) {
            stream << " sret " << instruction.memoryReturnDest;
        }
        stream << "\n";
        return;
    case Op::Retrieve:
        stream << "\t" << instruction.result << " := RETRIEVE"
               << (instruction.memoryReturn ? " (sret)\n" : "\n");
        return;
    case Op::Return:
        stream << "\tRETURN " << instruction.arg0 << "\n";
        return;
    case Op::VoidReturn:
        stream << "\tRETURN\n";
        return;
    case Op::Truncate:
        stream << "\tTRUNC" << (instruction.signedAccess ? "S" : "Z") << instruction.imm * 8
               << " " << instruction.arg0 << "\n";
        return;
    case Op::VaStart:
        stream << "\tva_start(" << instruction.arg0 << ")\n";
        return;
    case Op::VaArg:
        stream << "\t" << instruction.result << " := va_arg(" << instruction.arg0 << ")\n";
        return;
    case Op::VaCopy:
        stream << "\tva_copy(" << instruction.arg0 << ", " << instruction.arg1 << ")\n";
        return;
    case Op::Bswap:
        stream << "\tBSWAP" << instruction.imm << " " << instruction.arg0
                << " -> " << instruction.result << "\n";
        return;
    case Op::Ctz:
        stream << "\t" << instruction.result << " := ctz(" << instruction.arg0 << ")\n";
        return;
    }
    throw std::logic_error { "print(Instruction): unhandled Op" };
}

void print(std::ostream& stream, const Procedure& procedure) {
    stream << "PROC ";
    if (!procedure.exported) {
        stream << "static ";
    }
    stream << procedure.name;
    if (procedure.variadic) {
        stream << " variadic";
    }
    if (procedure.memoryReturn) {
        stream << " sret";
    }
    stream << "\n";
    for (const auto& instruction : procedure.body) {
        print(stream, instruction);
    }
    stream << "ENDPROC " << procedure.name << "\n";
}

void print(std::ostream& stream, const IntermediateRepresentation& ir) {
    for (const auto& procedure : ir.procedures) {
        print(stream, procedure);
    }
}

std::string toString(const Instruction& instruction) {
    std::ostringstream stream;
    print(stream, instruction);
    return stream.str();
}

std::string toString(const Procedure& procedure) {
    std::ostringstream stream;
    print(stream, procedure);
    return stream.str();
}

std::string toString(const IntermediateRepresentation& ir) {
    std::ostringstream stream;
    print(stream, ir);
    return stream.str();
}

std::ostream& operator<<(std::ostream& stream, const Instruction& instruction) {
    print(stream, instruction);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Procedure& procedure) {
    print(stream, procedure);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const IntermediateRepresentation& ir) {
    print(stream, ir);
    return stream;
}

} // namespace codegen
