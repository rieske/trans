#ifndef CODEGEN_INSTRUCTION_H_
#define CODEGEN_INSTRUCTION_H_

#include <ostream>
#include <string>
#include <vector>

#include "codegen/JumpCondition.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"
#include "symbols/BuiltinOpKind.h"

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
    AssignConstant,
    AssignLabelAddress,
    LvalueAssign,
    AddressOf,
    Dereference,
    IndexAddress,
    FieldAddress,
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
    Truncate,
    BuiltinOp,
    VaStart,
    VaArg,
    VaCopy,
    VaEnd,
};

struct ProcedureFrame {
    std::vector<Value> locals;
    std::vector<Value> arguments;
};

// Sparse fields: meaning depends on op (see ir:: builders and emit/print).
struct Instruction {
    Op op {};
    std::string arg0;
    std::string arg1;
    std::string result;
    int imm { 0 };
    JumpCondition cond { JumpCondition::UNCONDITIONAL };
    symbols::AddressBaseMode baseMode { symbols::AddressBaseMode::LeaObject };
    bool callIndirect { false };
    // Div/Mod: unsigned forms.
    bool unsignedArith { false };
    // Shr: logical (SHR) vs arithmetic (SAR).
    bool logicalShift { false };
    // Dereference / LvalueAssign / VaArg access width.
    int accessSizeBytes { 8 };
    // Dereference / Truncate / VaArg signedness.
    bool signedAccess { true };
    // VaArg: floating vs integer slot.
    bool floatingAccess { false };
    // Call sret destination symbol (empty if none).
    std::string memoryReturnDest;
    // Retrieve: true when preceding Call used sret into result.
    bool memoryReturn { false };
    // BuiltinOp kind when op == BuiltinOp.
    symbols::BuiltinOpKind builtinKind { symbols::BuiltinOpKind::Bswap16 };
};

struct Procedure {
    std::string name;
    ProcedureFrame frame;
    std::vector<Instruction> body;
    bool variadic { false };
    bool isStatic { false };
    bool memoryReturn { false };
};

struct IntermediateRepresentation {
    std::vector<Procedure> procedures;
};

bool instructionTransfersControl(const Instruction& instruction);

void print(std::ostream& stream, const Instruction& instruction);
void print(std::ostream& stream, const Procedure& procedure);
void print(std::ostream& stream, const IntermediateRepresentation& ir);

std::string toString(const Instruction& instruction);
std::string toString(const Procedure& procedure);
std::string toString(const IntermediateRepresentation& ir);

std::ostream& operator<<(std::ostream& stream, const Instruction& instruction);
std::ostream& operator<<(std::ostream& stream, const Procedure& procedure);
std::ostream& operator<<(std::ostream& stream, const IntermediateRepresentation& ir);

} // namespace codegen

#include "codegen/IrBuilders.h"

#endif // CODEGEN_INSTRUCTION_H_
