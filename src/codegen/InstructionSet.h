#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>
#include <map>
#include <vector>

#include "GlobalVariable.h"
#include "MemoryOperand.h"
#include "Sse.h"

namespace codegen {

class Register;

class InstructionSet {
public:
    virtual ~InstructionSet() = default;

    virtual std::string preamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables = {},
            const std::vector<std::string>& externalFunctions = {},
            const std::vector<std::string>& definedFunctions = {}) const = 0;

    virtual std::string call(std::string procedureName) const = 0;
    virtual std::string callPlt(std::string procedureName) const = 0;
    virtual std::string callIndirect(const Register& target) const = 0;
    virtual std::string loadGot(std::string symbolName, const Register& target) const = 0;

    virtual std::string push(const Register& reg) const = 0;
    virtual std::string pop(const Register& reg) const = 0;

    virtual std::string lea(const MemoryOperand& source, const Register& target) const = 0;

    virtual std::string add(const Register& reg, int constant) const = 0;
    virtual std::string sub(const Register& reg, int constant) const = 0;

    virtual std::string not_(const Register& reg) const = 0;

    virtual std::string mov(const Register& from, const MemoryOperand& destination) const = 0;
    virtual std::string mov(const Register& from, const Register& to) const = 0;
    virtual std::string mov(const MemoryOperand& source, const Register& to) const = 0;
    virtual std::string mov(std::string constant, const MemoryOperand& destination) const = 0;
    virtual std::string mov(std::string constant, const Register& to) const = 0;

    // Typed memory load/store (1/2/4/8) and subreg extend — dialect-owned.
    virtual std::string load(const MemoryOperand& source, const Register& dest, int sizeBytes,
            bool isSigned) const = 0;
    virtual std::string store(const Register& source, const MemoryOperand& dest, int sizeBytes) const = 0;
    virtual std::string extend(const Register& reg, int sizeBytes, bool isSigned) const = 0;
    virtual std::string storeImm(const MemoryOperand& dest, long long imm, int sizeBytes) const = 0;

    // Scalar SSE (SysV xmm0..7): width selects ss/movd vs sd/movq.
    virtual std::string sseGprXmm(SseGprXmmDir dir, SseWidth width, const Register& gpr,
            int xmmIndex) const = 0;
    // Low 64 bits of xmm to memory (va_list reg_save_area).
    virtual std::string sseXmmToMem(int xmmIndex, const MemoryOperand& dest) const = 0;
    virtual std::string sseCvtIntToXmm(const Register& gpr, int xmmIndex, SseWidth dest) const = 0;
    virtual std::string sseCvtTruncToGpr(int xmmIndex, const Register& gpr, SseWidth src) const = 0;
    virtual std::string sseCvtFloat(SseWidth from, SseWidth to, int srcXmm, int dstXmm) const = 0;
    virtual std::string sseBin(SseBin op, SseWidth width, int dstXmm, int srcXmm) const = 0;

    virtual std::string cqo() const = 0;
    virtual std::string bswap(const Register& reg, int sizeBytes) const = 0;
    virtual std::string bsf(const Register& reg) const = 0;
    virtual std::string shrImm(const Register& reg, int amount) const = 0;

    virtual std::string cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const = 0;
    virtual std::string cmp(const Register& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const Register& argument, int constant) const = 0;
    virtual std::string cmp(const MemoryOperand& leftArgument, int constant) const = 0;

    virtual std::string label(std::string name) const = 0;
    virtual std::string jmp(std::string label) const = 0;
    virtual std::string je(std::string label) const = 0;
    virtual std::string jne(std::string label) const = 0;
    virtual std::string jg(std::string label) const = 0; // signed >
    virtual std::string jl(std::string label) const = 0; // signed <
    virtual std::string jge(std::string label) const = 0; // signed >=
    virtual std::string jle(std::string label) const = 0; // signed <=
    virtual std::string ja(std::string label) const = 0; // unsigned >
    virtual std::string jb(std::string label) const = 0; // unsigned <
    virtual std::string jae(std::string label) const = 0; // unsigned >=
    virtual std::string jbe(std::string label) const = 0; // unsigned <=

    virtual std::string leave() const = 0;
    virtual std::string ret() const = 0;

    virtual std::string xor_(const Register& operand, const Register& result) const = 0;
    virtual std::string xor_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string or_(const Register& operand, const Register& result) const = 0;
    virtual std::string or_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string and_(const Register& operand, const Register& result) const = 0;
    virtual std::string and_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string shl(const Register& result) const = 0;
    // Logical right shift (unsigned >>): zero-fill.
    virtual std::string shr(const Register& result) const = 0;
    // Arithmetic right shift (signed >>): sign-extend.
    virtual std::string sar(const Register& result) const = 0;

    virtual std::string add(const Register& operand, const Register& result) const = 0;
    virtual std::string add(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string sub(const Register& operand, const Register& result) const = 0;
    virtual std::string sub(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string imul(const Register& operand) const = 0;
    virtual std::string imul(const MemoryOperand& operand) const = 0;

    virtual std::string idiv(const Register& operand) const = 0;
    virtual std::string idiv(const MemoryOperand& operand) const = 0;
    // Unsigned divide (div); idiv traps on quotients with high bit set (SIZE_MAX / n).
    virtual std::string div(const Register& operand) const = 0;
    virtual std::string div(const MemoryOperand& operand) const = 0;

    virtual std::string inc(const Register& operand) const = 0;
    virtual std::string inc(const MemoryOperand& operand) const = 0;

    virtual std::string dec(const Register& operand) const = 0;
    virtual std::string dec(const MemoryOperand& operand) const = 0;

    virtual std::string neg(const Register& operand) const = 0;
};

} // namespace codegen

#endif // INSTRUCTIONSET_H_
