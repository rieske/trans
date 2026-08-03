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

// Memory access width for typed load/store/extend/bswap. Public load() converts
// a C size in bytes to this; dialects implement one hook per op.
enum class AccessWidth { B1, B2, B4, B8 };

// GPR ALU width. 32-bit ops wrap (SHA-1 / unsigned int); 64-bit is the default
// for pointers, size_t, and long. No other widths.
enum class GprWidth { W32, W64 };

inline int gprWidthBytes(GprWidth width) {
    return width == GprWidth::W32 ? 4 : 8;
}

class InstructionSet {
public:
    virtual ~InstructionSet() = default;

    std::string preamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables = {},
            const std::vector<std::string>& externalSymbols = {}) const;

    // Identifier-token spelling of a linker symbol. ELF name is unchanged.
    // Labels, operands, relocs, jumps, and calls use this. globl/extern do not.
    virtual std::string asmSymbol(const std::string& name) const;

    // ELF name as written after global/extern. Not an identifier token.
    virtual std::string globl(const std::string& name) const = 0;
    virtual std::string externDirective(const std::string& name) const = 0;

    std::string call(std::string procedureName) const;
    virtual std::string callPlt(std::string procedureName) const = 0;
    virtual std::string callIndirect(const Register& target) const = 0;
    virtual std::string loadGot(std::string symbolName, const Register& target) const = 0;

    virtual std::string push(const Register& reg) const = 0;
    virtual std::string pop(const Register& reg) const = 0;

    virtual std::string lea(const MemoryOperand& source, const Register& target) const = 0;

    virtual std::string add(const Register& reg, int constant, GprWidth width) const = 0;
    virtual std::string sub(const Register& reg, int constant, GprWidth width) const = 0;

    virtual std::string not_(const Register& reg, GprWidth width) const = 0;

    virtual std::string mov(const Register& from, const MemoryOperand& destination) const = 0;
    virtual std::string mov(const Register& from, const Register& to) const = 0;
    virtual std::string mov(const MemoryOperand& source, const Register& to) const = 0;
    virtual std::string mov(std::string constant, const MemoryOperand& destination) const = 0;
    virtual std::string mov(std::string constant, const Register& to) const = 0;

    // Typed memory load/store (1/2/4/8) and subreg extend. Size dispatch is shared;
    // dialects implement the width-specific spellings.
    std::string load(const MemoryOperand& source, const Register& dest, int sizeBytes,
            bool isSigned) const;
    std::string store(const Register& source, const MemoryOperand& dest, int sizeBytes) const;
    std::string extend(const Register& reg, int sizeBytes, bool isSigned) const;
    std::string storeImm(const MemoryOperand& dest, long long imm, int sizeBytes) const;

    // Scalar SSE (SysV xmm0..7): width selects ss/movd vs sd/movq.
    virtual std::string sseGprXmm(SseGprXmmDir dir, SseWidth width, const Register& gpr,
            int xmmIndex) const = 0;
    // Low 64 bits of xmm to memory (va_list reg_save_area).
    virtual std::string sseXmmToMem(int xmmIndex, const MemoryOperand& dest) const = 0;
    virtual std::string sseCvtIntToXmm(const Register& gpr, int xmmIndex, SseWidth dest) const = 0;
    virtual std::string sseCvtTruncToGpr(int xmmIndex, const Register& gpr, SseWidth src) const = 0;
    virtual std::string sseCvtFloat(SseWidth from, SseWidth to, int srcXmm, int dstXmm) const = 0;
    virtual std::string sseBin(SseBin op, SseWidth width, int dstXmm, int srcXmm) const = 0;
    virtual std::string sseUcomi(SseWidth width, int leftXmm, int rightXmm) const = 0;

    virtual std::string cqo() const = 0;
    virtual std::string shrImm(const Register& reg, int amount) const = 0;

    virtual std::string cmp(const Register& leftArgument, const Register& rightArgument,
            GprWidth width) const = 0;
    virtual std::string cmp(const Register& argument, int constant, GprWidth width) const = 0;

    std::string label(std::string name) const;
    std::string jmp(std::string label) const;
    std::string je(std::string label) const;
    std::string jne(std::string label) const;
    std::string jg(std::string label) const; // signed >
    std::string jl(std::string label) const; // signed <
    std::string jge(std::string label) const; // signed >=
    std::string jle(std::string label) const; // signed <=
    std::string ja(std::string label) const; // unsigned >
    std::string jb(std::string label) const; // unsigned <
    std::string jae(std::string label) const; // unsigned >=
    std::string jbe(std::string label) const; // unsigned <=

    std::string leave() const;
    std::string ret() const;

    virtual std::string xor_(const Register& operand, const Register& result, GprWidth width) const = 0;

    virtual std::string or_(const Register& operand, const Register& result, GprWidth width) const = 0;

    virtual std::string and_(const Register& operand, const Register& result, GprWidth width) const = 0;

    virtual std::string shl(const Register& result, GprWidth width) const = 0;
    virtual std::string shr(const Register& result, GprWidth width) const = 0;
    virtual std::string sar(const Register& result, GprWidth width) const = 0;
    virtual std::string shld(const Register& source, const Register& dest) const = 0;
    virtual std::string shrd(const Register& source, const Register& dest) const = 0;

    virtual std::string add(const Register& operand, const Register& result, GprWidth width) const = 0;
    virtual std::string adc(const Register& operand, const Register& result) const = 0;

    virtual std::string sub(const Register& operand, const Register& result, GprWidth width) const = 0;
    virtual std::string sbb(const Register& operand, const Register& result) const = 0;

    virtual std::string imul(const Register& operand, GprWidth width) const = 0;

    virtual std::string idiv(const Register& operand, GprWidth width) const = 0;
    virtual std::string div(const Register& operand, GprWidth width) const = 0;

    virtual std::string cdq() const = 0;
    virtual std::string inc(const Register& operand, GprWidth width) const = 0;
    std::string inc(const MemoryOperand& operand, int sizeBytes) const;

    virtual std::string dec(const Register& operand, GprWidth width) const = 0;
    std::string dec(const MemoryOperand& operand, int sizeBytes) const;

    virtual std::string neg(const Register& operand, GprWidth width) const = 0;
    std::vector<std::string> bswap(const Register& operand, int widthBytes) const;
    virtual std::string ctz(const Register& operand, int widthBytes) const = 0;

    virtual std::string loadX87(const MemoryOperand& source, int sizeBytes = 16) const = 0;
    virtual std::string storeX87(const MemoryOperand& dest, int sizeBytes = 16) const = 0;
    virtual std::string fild(const MemoryOperand& source, int sizeBytes) const = 0;
    virtual std::string fisttp(const MemoryOperand& dest, int sizeBytes) const = 0;
    virtual std::string faddp() const = 0;
    virtual std::string fsubp() const = 0;
    virtual std::string fmulp() const = 0;
    virtual std::string fdivp() const = 0;
    std::string fchs() const;
    std::string fldz() const;
    virtual std::string fucomip() const = 0;
    virtual std::string fstpSt0() const = 0;

protected:
    virtual std::string preamblePrefix() const;
    virtual std::string globlDataLine(const std::string& name) const;
    std::vector<std::string> formattedDataOperands(const GlobalVariable& global) const;
    virtual std::string dataSectionHeader() const = 0;
    virtual std::string textSectionHeader() const = 0;
    virtual std::string constantLine(const std::string& name, const std::string& escapedValue) const = 0;
    virtual std::string alignDirective(int bytes) const = 0;
    virtual std::string dataObjectLines(const GlobalVariable& global) const = 0;

    virtual std::string loadW(const MemoryOperand& source, const Register& dest, AccessWidth width,
            bool isSigned) const = 0;
    virtual std::string storeW(const Register& source, const MemoryOperand& dest, AccessWidth width) const = 0;
    virtual std::string extendW(const Register& reg, AccessWidth width, bool isSigned) const = 0;
    virtual std::string storeImmW(const MemoryOperand& dest, long long imm, AccessWidth width) const = 0;
    virtual std::vector<std::string> bswapW(const Register& operand, AccessWidth width) const = 0;
    virtual std::string incW(const MemoryOperand& operand, AccessWidth width) const = 0;
    virtual std::string decW(const MemoryOperand& operand, AccessWidth width) const = 0;
};

} // namespace codegen

#endif // INSTRUCTIONSET_H_
