#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>
#include <map>
#include <vector>

#include "GlobalVariable.h"
#include "MemoryOperand.h"

namespace codegen {

class Register;

class InstructionSet {
public:
    virtual ~InstructionSet() = default;

    std::string preamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables = {},
            const std::vector<std::string>& externalFunctions = {}) const;

    virtual std::string globl(const std::string& name) const = 0;
    virtual std::string externDirective(const std::string& name) const = 0;

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

    // SSE float/double bits in gpr / xmm0..xmm7 (SysV floating ABI).
    virtual std::string movqGprToXmm(const Register& gpr, int xmmIndex) const = 0;
    virtual std::string movqXmmToGpr(int xmmIndex, const Register& gpr) const = 0;
    virtual std::string movdGprToXmm(const Register& gpr, int xmmIndex) const = 0;
    virtual std::string movdXmmToGpr(int xmmIndex, const Register& gpr) const = 0;
    virtual std::string movDword(const MemoryOperand& source, const Register& dest) const = 0;
    virtual std::string movDword(const Register& source, const MemoryOperand& dest) const = 0;
    virtual std::string cvtsi2sd(const Register& gpr, int xmmIndex) const = 0;
    virtual std::string cvttsd2si(int xmmIndex, const Register& gpr) const = 0;
    virtual std::string cvtsi2ss(const Register& gpr, int xmmIndex) const = 0;
    virtual std::string cvttss2si(int xmmIndex, const Register& gpr) const = 0;
    virtual std::string cvtss2sd(int srcXmm, int dstXmm) const = 0;
    virtual std::string cvtsd2ss(int srcXmm, int dstXmm) const = 0;
    virtual std::string addsd(int dstXmm, int srcXmm) const = 0;
    virtual std::string subsd(int dstXmm, int srcXmm) const = 0;
    virtual std::string mulsd(int dstXmm, int srcXmm) const = 0;
    virtual std::string divsd(int dstXmm, int srcXmm) const = 0;
    virtual std::string addss(int dstXmm, int srcXmm) const = 0;
    virtual std::string subss(int dstXmm, int srcXmm) const = 0;
    virtual std::string mulss(int dstXmm, int srcXmm) const = 0;
    virtual std::string divss(int dstXmm, int srcXmm) const = 0;

    virtual std::string cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const = 0;
    virtual std::string cmp(const Register& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const Register& argument, int constant) const = 0;
    virtual std::string cmp(const MemoryOperand& leftArgument, int constant) const = 0;

    virtual std::string label(std::string name) const = 0;
    virtual std::string jmp(std::string label) const = 0;
    virtual std::string je(std::string label) const = 0;
    virtual std::string jne(std::string label) const = 0;
    virtual std::string jg(std::string label) const = 0; // signed
    virtual std::string jl(std::string label) const = 0; // signed
    virtual std::string jge(std::string label) const = 0; // signed
    virtual std::string jle(std::string label) const = 0; // signed
    virtual std::string ja(std::string label) const = 0; // unsigned
    virtual std::string jb(std::string label) const = 0; // unsigned

    virtual std::string syscall() const = 0;
    virtual std::string leave() const = 0;
    virtual std::string ret() const = 0;

    virtual std::string xor_(const Register& operand, const Register& result) const = 0;
    virtual std::string xor_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string or_(const Register& operand, const Register& result) const = 0;
    virtual std::string or_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string and_(const Register& operand, const Register& result) const = 0;
    virtual std::string and_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string shl(const Register& result) const = 0;
    //virtual std::string shl(std::string constant, const Register& result) const = 0;
    virtual std::string shr(const Register& result) const = 0;
    //virtual std::string shr(std::string constant, const Register& result) const = 0;
    virtual std::string lshr(const Register& result) const = 0;
    virtual std::string shld(const Register& source, const Register& dest) const = 0;
    virtual std::string shrd(const Register& source, const Register& dest) const = 0;

    virtual std::string add(const Register& operand, const Register& result) const = 0;
    virtual std::string add(const MemoryOperand& operand, const Register& result) const = 0;
    virtual std::string adc(const Register& operand, const Register& result) const = 0;

    virtual std::string sub(const Register& operand, const Register& result) const = 0;
    virtual std::string sub(const MemoryOperand& operand, const Register& result) const = 0;
    virtual std::string sbb(const Register& operand, const Register& result) const = 0;

    virtual std::string imul(const Register& operand) const = 0;
    virtual std::string imul(const MemoryOperand& operand) const = 0;

    virtual std::string idiv(const Register& operand) const = 0;
    virtual std::string idiv(const MemoryOperand& operand) const = 0;

    // Sign-extend RAX into RDX:RAX before signed idiv (not xor rdx,rdx).
    virtual std::string cqo() const = 0;

    virtual std::string inc(const Register& operand) const = 0;
    virtual std::string inc(const MemoryOperand& operand) const = 0;

    virtual std::string dec(const Register& operand) const = 0;
    virtual std::string dec(const MemoryOperand& operand) const = 0;

    virtual std::string neg(const Register& operand) const = 0;
    virtual std::vector<std::string> bswap(const Register& operand, int widthBytes) const = 0;

    virtual std::string loadX87(const MemoryOperand& source) const = 0;
    virtual std::string storeX87(const MemoryOperand& dest) const = 0;

    virtual std::string loadByteSignExtend(const Register& address, const Register& dest) const = 0;
    virtual std::string loadByteZeroExtend(const Register& address, const Register& dest) const = 0;
    virtual std::string loadWordSignExtend(const Register& address, const Register& dest) const = 0;
    virtual std::string loadWordZeroExtend(const Register& address, const Register& dest) const = 0;
    virtual std::string loadDwordSignExtend(const Register& address, const Register& dest) const = 0;
    virtual std::string storeByte(const Register& source, const Register& address) const = 0;
    virtual std::string storeDword(const Register& source, const Register& address) const = 0;

protected:
    virtual std::string preamblePrefix() const;
    virtual std::string globlDataLine(const std::string& name) const;
    virtual std::string dataSectionHeader() const = 0;
    virtual std::string textSectionHeader() const = 0;
    virtual std::string constantLine(const std::string& name, const std::string& escapedValue) const = 0;
    virtual std::string dataObjectLines(const GlobalVariable& global) const = 0;
};

} // namespace codegen

#endif // INSTRUCTIONSET_H_
