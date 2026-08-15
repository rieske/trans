#include "Instruction.h"

#include <sstream>
#include <stdexcept>
#include <string_view>

namespace codegen {

namespace {

std::string_view name(const IrStringTable& strings, int id) {
    if (id < 0) {
        return {};
    }
    return strings.get(id);
}

void printJump(std::ostream& stream, const Instruction& instruction, const IrStringTable& strings) {
    stream << "\t";
    switch (instruction.cond) {
    case JumpCondition::IF_EQUAL:
        stream << "JE ";
        break;
    case JumpCondition::IF_NOT_EQUAL:
        stream << "JNE ";
        break;
    case JumpCondition::IF_ABOVE:
        stream << "JA ";
        break;
    case JumpCondition::IF_BELOW:
        stream << "JB ";
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        stream << "JAE ";
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        stream << "JBE ";
        break;
    case JumpCondition::UNCONDITIONAL:
        stream << "GOTO ";
        break;
    default:
        throw std::logic_error { "printJump: unhandled JumpCondition" };
    }
    stream << name(strings, instruction.arg0) << "\n";
}

} // namespace

void print(std::ostream& stream, const Instruction& instruction, const IrStringTable& strings) {
    switch (instruction.op) {
    case Op::Add:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " + " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Sub:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " - " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Mul:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " * " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Div:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " / " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Mod:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " % " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::And:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " AND " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Or:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " OR " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Xor:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " XOR " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Shl:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " << " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::Shr:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << " >> " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::UnaryMinus:
        stream << "\t" << name(strings, instruction.result) << " := -" << name(strings, instruction.arg0) << "\n";
        return;
    case Op::UnaryNot:
        stream << "\t" << name(strings, instruction.result) << " := ~" << name(strings, instruction.arg0) << "\n";
        return;
    case Op::Inc:
        if (instruction.imm == 1) {
            stream << "\tINC " << name(strings, instruction.arg0) << "\n";
        } else {
            stream << "\t" << name(strings, instruction.arg0) << " := " << name(strings, instruction.arg0)
                   << " + " << instruction.imm << "\n";
        }
        return;
    case Op::Dec:
        if (instruction.imm == 1) {
            stream << "\tDEC " << name(strings, instruction.arg0) << "\n";
        } else {
            stream << "\t" << name(strings, instruction.arg0) << " := " << name(strings, instruction.arg0)
                   << " - " << instruction.imm << "\n";
        }
        return;
    case Op::Assign:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0) << "\n";
        return;
    case Op::Widen:
        stream << "\t" << name(strings, instruction.result) << " := widen "
               << name(strings, instruction.arg0) << "\n";
        return;
    case Op::AssignConstant:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0) << "\n";
        return;
    case Op::AssignLabelAddress:
        stream << "\t" << name(strings, instruction.result) << " := &" << name(strings, instruction.arg0) << "\n";
        return;
    case Op::LvalueAssign:
        stream << "\t*" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0) << "\n";
        return;
    case Op::AddressOf:
        stream << "\t" << name(strings, instruction.result) << " := &" << name(strings, instruction.arg0) << "\n";
        return;
    case Op::Dereference:
        stream << "\t" << name(strings, instruction.result) << " := *" << name(strings, instruction.arg0) << "\n";
        return;
    case Op::IndexAddress:
        stream << "\t" << name(strings, instruction.result) << " := &" << name(strings, instruction.arg0)
               << "[" << name(strings, instruction.arg1)
               << "] stride=" << instruction.imm
               << (symbols::addressBaseUsesLea(instruction.baseMode) ? " (array)\n" : " (ptr)\n");
        return;
    case Op::FieldAddress: {
        const char* op = symbols::addressBaseIsPointerValue(instruction.baseMode) ? "->" : ".";
        stream << "\t" << name(strings, instruction.result) << " := &("
               << name(strings, instruction.arg0) << op << instruction.imm << ")\n";
        return;
    }
    case Op::CopyPart:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << "[+" << instruction.imm << "]\n";
        return;
    case Op::PointerOffset:
        stream << "\t" << name(strings, instruction.result) << " := " << name(strings, instruction.arg0)
               << (instruction.pointerSubtract ? " - " : " + ") << name(strings, instruction.arg1);
        if (instruction.imm != 1) {
            stream << "*" << instruction.imm;
        }
        stream << " (ptr)\n";
        return;
    case Op::PointerDiff:
        stream << "\t" << name(strings, instruction.result) << " := (" << name(strings, instruction.arg0)
               << " - " << name(strings, instruction.arg1) << ")";
        if (instruction.imm != 1) {
            stream << " /" << instruction.imm;
        }
        stream << " (ptrdiff)\n";
        return;
    case Op::FunctionAddress:
        stream << "\t" << name(strings, instruction.result) << " := &" << name(strings, instruction.arg0)
               << " (function)\n";
        return;
    case Op::ValueCompare:
        stream << "\tCMP " << name(strings, instruction.arg0) << ", " << name(strings, instruction.arg1) << "\n";
        return;
    case Op::ZeroCompare:
        stream << "\tCMP " << name(strings, instruction.arg0) << ", 0\n";
        return;
    case Op::Jump:
        printJump(stream, instruction, strings);
        return;
    case Op::Label:
        stream << name(strings, instruction.arg0) << ":\n";
        return;
    case Op::Argument:
        stream << "\tPARAM " << name(strings, instruction.arg0) << "\n";
        return;
    case Op::Call:
        if (instruction.callIndirect) {
            stream << "\tCALL *" << name(strings, instruction.arg0);
        } else {
            stream << "\tCALL " << name(strings, instruction.arg0);
        }
        if (instruction.memoryReturnDest >= 0) {
            stream << " sret " << name(strings, instruction.memoryReturnDest);
        }
        stream << "\n";
        return;
    case Op::Retrieve:
        stream << "\tRETRIEVE " << name(strings, instruction.result);
        if (instruction.memoryReturn) {
            stream << " (sret)";
        }
        stream << "\n";
        return;
    case Op::Return:
        stream << "\tRETURN " << name(strings, instruction.arg0) << "\n";
        return;
    case Op::VoidReturn:
        stream << "\tRETURN\n";
        return;
    case Op::VaStart:
        stream << "\tVA_START " << name(strings, instruction.arg0);
        if (instruction.arg1 >= 0) {
            stream << ", " << name(strings, instruction.arg1);
        }
        stream << "\n";
        return;
    case Op::VaArg:
        stream << "\tVA_ARG " << name(strings, instruction.arg0) << " -> "
               << name(strings, instruction.result) << "\n";
        return;
    case Op::VaCopy:
        stream << "\tVA_COPY " << name(strings, instruction.arg0) << ", "
               << name(strings, instruction.arg1) << "\n";
        return;
    case Op::VaEnd:
        stream << "\tVA_END\n";
        return;
    case Op::Bswap:
        stream << "\tBSWAP" << instruction.imm << " " << name(strings, instruction.arg0)
               << " -> " << name(strings, instruction.result) << "\n";
        return;
    case Op::Ctz:
        stream << "\tCTZ" << instruction.imm << " " << name(strings, instruction.arg0)
               << " -> " << name(strings, instruction.result) << "\n";
        return;
    case Op::Alloca:
        stream << "\tALLOCA " << name(strings, instruction.arg0) << " -> "
               << name(strings, instruction.result) << "\n";
        return;
    }
    throw std::logic_error { "print(Instruction): unhandled Op" };
}

void print(std::ostream& stream, const Procedure& procedure, const IrStringTable& strings) {
    stream << "PROC " << strings.get(procedure.name);
    if (procedure.memoryReturn) {
        stream << " sret";
    }
    if (procedure.variadic) {
        stream << " variadic";
    }
    stream << "\n";
    for (const auto& instruction : procedure.body) {
        print(stream, instruction, strings);
    }
    stream << "ENDPROC " << strings.get(procedure.name) << "\n";
}

void print(std::ostream& stream, const IntermediateRepresentation& ir) {
    for (const auto& procedure : ir.procedures) {
        print(stream, procedure, ir.strings);
    }
}

std::string toString(const IntermediateRepresentation& ir) {
    std::ostringstream stream;
    print(stream, ir);
    return stream.str();
}

std::ostream& operator<<(std::ostream& stream, const IntermediateRepresentation& ir) {
    print(stream, ir);
    return stream;
}

} // namespace codegen
