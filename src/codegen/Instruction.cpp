#include "Instruction.h"

#include <stdexcept>

namespace codegen {

bool instructionTransfersControl(const Instruction& instruction) {
    switch (instruction.op) {
    case Op::Jump:
    case Op::Return:
    case Op::VoidReturn:
        return true;
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::Div:
    case Op::Mod:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::Shl:
    case Op::Shr:
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Inc:
    case Op::Dec:
    case Op::Assign:
    case Op::AssignConstant:
    case Op::AssignLabelAddress:
    case Op::LvalueAssign:
    case Op::AddressOf:
    case Op::Dereference:
    case Op::IndexAddress:
    case Op::FieldAddress:
    case Op::PointerOffset:
    case Op::PointerDiff:
    case Op::FunctionAddress:
    case Op::ValueCompare:
    case Op::ZeroCompare:
    case Op::Label:
    case Op::Argument:
    case Op::Call:
    case Op::Retrieve:
    case Op::VaStart:
    case Op::VaArg:
    case Op::VaEnd:
    case Op::VaCopy:
        return false;
    }
    throw std::logic_error { "instructionTransfersControl: unhandled Op" };
}

} // namespace codegen
