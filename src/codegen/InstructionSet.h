#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>
#include <string_view>
#include <map>
#include <vector>

#include "GlobalVariable.h"
#include "MemoryOperand.h"

namespace codegen {

class Register;

enum class AsmSyntax {
    Intel,
    Att
};

class InstructionSet {
public:
    explicit InstructionSet(AsmSyntax syntax);

    std::string preamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables = {},
            const std::vector<std::string>& externalSymbols = {}) const;

    // Identifier-token spelling of a linker symbol. ELF name is unchanged.
    // Labels, operands, relocs, jumps, and calls use this. globl/extern do not.
    std::string asmSymbol(const std::string& name) const;

    // ELF name as written after global/extern. Not an identifier token.
    std::string globl(const std::string& name) const;
    std::string externDirective(const std::string& name) const;

    std::string call(std::string procedureName) const;
    std::string callPlt(std::string procedureName) const;
    std::string callIndirect(const Register& target) const;
    std::string loadGot(std::string symbolName, const Register& target) const;

    std::string push(const Register& reg) const;
    std::string pop(const Register& reg) const;

    std::string lea(const MemoryOperand& source, const Register& target) const;

    std::string add(const Register& reg, int constant) const;
    std::string sub(const Register& reg, int constant) const;

    std::string not_(const Register& reg, int widthBytes = 8) const;

    std::string mov(const Register& from, const MemoryOperand& destination) const;
    std::string mov(const Register& from, const Register& to) const;
    std::string mov(const MemoryOperand& source, const Register& to) const;
    std::string mov(std::string constant, const MemoryOperand& destination) const;
    std::string mov(std::string constant, const Register& to) const;

    // SSE float/double bits in gpr / xmm0..xmm7 (SysV floating ABI).
    std::string movqGprToXmm(const Register& gpr, int xmmIndex) const;
    std::string movqXmmToGpr(int xmmIndex, const Register& gpr) const;
    std::string movdGprToXmm(const Register& gpr, int xmmIndex) const;
    std::string movdXmmToGpr(int xmmIndex, const Register& gpr) const;
    std::string movDword(const MemoryOperand& source, const Register& dest) const;
    std::string movDword(const Register& source, const MemoryOperand& dest) const;
    std::string cvtsi2sd(const Register& gpr, int xmmIndex) const;
    std::string cvttsd2si(int xmmIndex, const Register& gpr) const;
    std::string cvtsi2ss(const Register& gpr, int xmmIndex) const;
    std::string cvttss2si(int xmmIndex, const Register& gpr) const;
    std::string cvtss2sd(int srcXmm, int dstXmm) const;
    std::string cvtsd2ss(int srcXmm, int dstXmm) const;
    std::string addsd(int dstXmm, int srcXmm) const;
    std::string subsd(int dstXmm, int srcXmm) const;
    std::string mulsd(int dstXmm, int srcXmm) const;
    std::string divsd(int dstXmm, int srcXmm) const;
    std::string addss(int dstXmm, int srcXmm) const;
    std::string ucomiss(int leftXmm, int rightXmm) const;
    std::string ucomisd(int leftXmm, int rightXmm) const;
    std::string subss(int dstXmm, int srcXmm) const;
    std::string mulss(int dstXmm, int srcXmm) const;
    std::string divss(int dstXmm, int srcXmm) const;

    std::string cmp(const Register& leftArgument, const Register& rightArgument,
            int widthBytes = 8) const;
    std::string cmp(const Register& argument, int constant, int widthBytes = 8) const;

    std::string label(std::string name) const;
    std::string jmp(std::string label) const;
    std::string je(std::string label) const;
    std::string jns(std::string label) const;
    std::string jne(std::string label) const;
    std::string jg(std::string label) const;
    std::string jl(std::string label) const;
    std::string jge(std::string label) const;
    std::string jle(std::string label) const;
    std::string ja(std::string label) const;
    std::string jp(std::string label) const;
    std::string jb(std::string label) const;
    std::string jae(std::string label) const;
    std::string jbe(std::string label) const;

    std::string leave() const;
    std::string ret() const;

    std::string xor_(const Register& operand, const Register& result, int widthBytes = 8) const;

    std::string or_(const Register& operand, const Register& result, int widthBytes = 8) const;

    std::string and_(const Register& operand, const Register& result, int widthBytes = 8) const;

    std::string shl(const Register& result, int widthBytes = 8) const;
    // Signed >>. Arithmetic shift; logical shr would turn negatives positive.
    std::string shr(const Register& result, int widthBytes = 8) const;
    std::string lshr(const Register& result, int widthBytes = 8) const;
    std::string lshrImm(const Register& result, int amount) const;
    std::string andImm(const Register& result, int value) const;
    std::string shld(const Register& source, const Register& dest) const;
    std::string shrd(const Register& source, const Register& dest) const;

    std::string add(const Register& operand, const Register& result, int widthBytes = 8) const;
    std::string adc(const Register& operand, const Register& result) const;

    std::string sub(const Register& operand, const Register& result, int widthBytes = 8) const;
    std::string sbb(const Register& operand, const Register& result) const;

    std::string imul(const Register& operand, int widthBytes = 8) const;

    std::string idiv(const Register& operand, int widthBytes = 8) const;
    std::string div(const Register& operand, int widthBytes = 8) const;

    // Sign-extend EAX/RAX into EDX:EAX or RDX:RAX before signed idiv.
    std::string cdq() const;
    std::string cqo() const;

    std::string inc(const Register& operand, int widthBytes = 8) const;

    std::string dec(const Register& operand, int widthBytes = 8) const;

    std::string neg(const Register& operand, int widthBytes = 8) const;
    std::vector<std::string> bswap(const Register& operand, int widthBytes) const;
    std::string ctz(const Register& operand, int widthBytes) const;

    std::string loadX87(const MemoryOperand& source, int sizeBytes = 16) const;
    std::string storeX87(const MemoryOperand& dest, int sizeBytes = 16) const;
    std::string fild(const MemoryOperand& source, int sizeBytes) const;
    std::string fisttp(const MemoryOperand& dest, int sizeBytes) const;
    std::string faddp() const;
    std::string fsubp() const;
    std::string fmulp() const;
    std::string fdivp() const;
    std::string fchs() const;
    std::string fldz() const;
    std::string fucomip() const;
    std::string fstpSt0() const;

    std::string loadByteSignExtend(const MemoryOperand& source, const Register& dest) const;
    std::string loadByteZeroExtend(const MemoryOperand& source, const Register& dest) const;
    std::string loadWordSignExtend(const MemoryOperand& source, const Register& dest) const;
    std::string loadWordZeroExtend(const MemoryOperand& source, const Register& dest) const;
    std::string loadDwordSignExtend(const MemoryOperand& source, const Register& dest) const;
    std::string storeByte(const Register& source, const MemoryOperand& dest) const;
    std::string storeWord(const Register& source, const MemoryOperand& dest) const;

private:
    std::string preamblePrefix() const;
    std::string globlDataLine(const std::string& name) const;
    std::string dataSectionHeader() const;
    std::string bssSectionHeader() const;
    std::string textSectionHeader() const;
    std::string constantLine(const std::string& name, const std::string& escapedValue) const;
    std::string alignDirective(int bytes) const;
    std::string dataObjectLines(const GlobalVariable& global) const;
    std::string bssObjectLines(const GlobalVariable& global) const;
    std::string dataOperandText(const symbols::StaticInitValue& value) const;
    std::string joinedDataOperands(const GlobalVariable& global) const;

    AsmSyntax syntax_;

    bool att() const;
    std::string decorateReg(const std::string& name) const;
    std::string reg(const Register& r) const;
    std::string reg(const Register& r, int widthBytes) const;
    std::string imm(int value) const;
    std::string immText(const std::string& constant) const;
    std::string mem(const MemoryOperand& operand) const;
    std::string sizedMem(const char* intelSize, const MemoryOperand& operand) const;
    std::string mnemonic(const char* op, int widthBytes) const;
    std::string extendLoad(std::string_view intelOp, std::string_view attOp, const char* intelSize,
            const MemoryOperand& source, const Register& dest) const;
    std::string storeNarrow(const char* intelSize, std::string_view attOp, const std::string& narrowName,
            const MemoryOperand& dest) const;
    std::string binary(const char* op, const std::string& dest, const std::string& src,
            int widthBytes) const;
    std::string doubleShift(const char* op, const Register& source, const Register& dest) const;
    std::string defined(const std::string& name, const std::string& body) const;
    std::string unary(const char* op, const std::string& operand, int widthBytes) const;
    std::string jump(const char* op, std::string label) const;
    std::string xmm(int index) const;
    std::string ordered(std::string_view op, const std::string& dest, const std::string& src) const;
    std::string ordered(std::string_view intelOp, std::string_view attOp, const std::string& dest,
            const std::string& src) const;
    std::string x87Mem(const char* intelOp, const char* attOp, int sizeBytes,
            const MemoryOperand& operand) const;
};

} // namespace codegen

#endif // INSTRUCTIONSET_H_
