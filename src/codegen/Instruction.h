#ifndef CODEGEN_INSTRUCTION_H_
#define CODEGEN_INSTRUCTION_H_

#include <ostream>
#include <string>
#include <vector>

#include "codegen/IrStringTable.h"
#include "codegen/JumpCondition.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"

namespace codegen {

enum class Op {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    And,
    Or,
    Xor,
    Shl,
    Shr,
    UnaryMinus,
    UnaryNot,
    Inc,
    Dec,
    Assign,
    Widen,
    AssignConstant,
    AssignLabelAddress,
    LvalueAssign,
    AddressOf,
    Dereference,
    IndexAddress,
    FieldAddress,
    CopyPart,
    PointerOffset,
    PointerDiff,
    FunctionAddress,
    ValueCompare,
    ZeroCompare,
    Jump,
    Label,
    Argument,
    Call,
    Retrieve,
    Return,
    VoidReturn,
    VaStart,
    VaArg,
    VaEnd,
    VaCopy,
    Bswap,
    Ctz,
    Alloca,
};

struct ProcedureFrame {
    std::vector<Value> locals;
    std::vector<Value> arguments;
};

// Sparse fields: meaning depends on op (see ir:: builders and emit/print).
// Value, label, constant, and function operands are ids into IrStringTable.
struct Instruction {
    Op op {};
    int arg0 { kNoSymbol };
    int arg1 { kNoSymbol };
    int result { kNoSymbol };
    int imm { 0 };
    JumpCondition cond { JumpCondition::UNCONDITIONAL };
    symbols::AddressBaseMode baseMode { symbols::AddressBaseMode::LeaObject };
    bool callIndirect { false };
    bool pointerSubtract { false };
    int memoryReturnDest { kNoSymbol };
    bool memoryReturn { false };
};

struct Procedure {
    int name { kNoSymbol };
    ProcedureFrame frame;
    std::vector<Instruction> body;
    bool memoryReturn { false };
    bool variadic { false };
    bool exported { true };
    int sretId { kNoSymbol };
    std::vector<int> vaGpHomes;
    std::vector<int> vaXmmHomes;
};

struct IntermediateRepresentation {
    IrStringTable strings;
    std::vector<Procedure> procedures;
};

void internProcedureTemps(IrStringTable& strings, Procedure& procedure);

bool instructionTransfersControl(const Instruction& instruction);

void print(std::ostream& stream, const Instruction& instruction, const IrStringTable& strings);
void print(std::ostream& stream, const Procedure& procedure, const IrStringTable& strings);
void print(std::ostream& stream, const IntermediateRepresentation& ir);

std::string toString(const IntermediateRepresentation& ir);

std::ostream& operator<<(std::ostream& stream, const IntermediateRepresentation& ir);

} // namespace codegen

#include "codegen/IrBuilders.h"

#endif // CODEGEN_INSTRUCTION_H_
