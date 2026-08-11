#ifndef INTEL_INSTRUCTIONSET_H_
#define INTEL_INSTRUCTIONSET_H_

#include <string>

#include "InstructionSet.h"

namespace codegen {

class IntelInstructionSet: public InstructionSet {
public:
    virtual ~IntelInstructionSet();

    std::string globl(const std::string& name) const override;
    std::string externDirective(const std::string& name) const override;

    std::string call(std::string procedureName) const override;
    std::string callPlt(std::string procedureName) const override;
    std::string callIndirect(const Register& target) const override;
    std::string loadGot(std::string symbolName, const Register& target) const override;

    std::string push(const Register& reg) const override;
    std::string pop(const Register& reg) const override;

    std::string add(const Register& reg, int constant) const override;
    std::string sub(const Register& reg, int constant) const override;

    std::string lea(const MemoryOperand& source, const Register& target) const override;

    std::string not_(const Register& reg) const override;

    std::string mov(const Register& from, const MemoryOperand& destination) const override;
    std::string mov(const Register& from, const Register& to) const override;
    std::string mov(const MemoryOperand& source, const Register& to) const override;
    std::string mov(std::string constant, const MemoryOperand& destination) const override;
    std::string mov(std::string constant, const Register& to) const override;

    std::string movqGprToXmm(const Register& gpr, int xmmIndex) const override;
    std::string movqXmmToGpr(int xmmIndex, const Register& gpr) const override;
    std::string movdGprToXmm(const Register& gpr, int xmmIndex) const override;
    std::string movdXmmToGpr(int xmmIndex, const Register& gpr) const override;
    std::string movDword(const MemoryOperand& source, const Register& dest) const override;
    std::string movDword(const Register& source, const MemoryOperand& dest) const override;
    std::string cvtsi2sd(const Register& gpr, int xmmIndex) const override;
    std::string cvttsd2si(int xmmIndex, const Register& gpr) const override;
    std::string cvtsi2ss(const Register& gpr, int xmmIndex) const override;
    std::string cvttss2si(int xmmIndex, const Register& gpr) const override;
    std::string cvtss2sd(int srcXmm, int dstXmm) const override;
    std::string cvtsd2ss(int srcXmm, int dstXmm) const override;
    std::string addsd(int dstXmm, int srcXmm) const override;
    std::string subsd(int dstXmm, int srcXmm) const override;
    std::string mulsd(int dstXmm, int srcXmm) const override;
    std::string divsd(int dstXmm, int srcXmm) const override;
    std::string addss(int dstXmm, int srcXmm) const override;
    std::string subss(int dstXmm, int srcXmm) const override;
    std::string mulss(int dstXmm, int srcXmm) const override;
    std::string divss(int dstXmm, int srcXmm) const override;

    std::string cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const override;
    std::string cmp(const Register& leftArgument, const Register& rightArgument) const override;
    std::string cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const override;
    std::string cmp(const Register& argument, int constant) const override;
    std::string cmp(const MemoryOperand& leftArgument, int constant) const override;

    std::string label(std::string name) const override;
    std::string jmp(std::string label) const override;
    std::string je(std::string label) const override;
    std::string jne(std::string label) const override;
    std::string jg(std::string label) const override;
    std::string jl(std::string label) const override;
    std::string jge(std::string label) const override;
    std::string jle(std::string label) const override;
    std::string ja(std::string label) const override;
    std::string jb(std::string label) const override;

    std::string syscall() const override;
    std::string leave() const override;
    std::string ret() const override;

    std::string xor_(const Register& operand, const Register& result) const override;
    std::string xor_(const MemoryOperand& operand, const Register& result) const override;

    std::string or_(const Register& operand, const Register& result) const override;
    std::string or_(const MemoryOperand& operand, const Register& result) const override;

    std::string and_(const Register& operand, const Register& result) const override;
    std::string and_(const MemoryOperand& operand, const Register& result) const override;

    std::string shl(const Register& result) const override;
    //std::string shl(std::string constant, const Register& result) const override;
    std::string shr(const Register& result) const override;
    //std::string shr(std::string constant, const Register& result) const override;
    std::string lshr(const Register& result) const override;
    std::string shld(const Register& source, const Register& dest) const override;
    std::string shrd(const Register& source, const Register& dest) const override;

    std::string add(const Register& operand, const Register& result) const override;
    std::string add(const MemoryOperand& operand, const Register& result) const override;
    std::string adc(const Register& operand, const Register& result) const override;

    std::string sub(const Register& operand, const Register& result) const override;
    std::string sub(const MemoryOperand& operand, const Register& result) const override;
    std::string sbb(const Register& operand, const Register& result) const override;

    std::string imul(const Register& operand) const override;
    std::string imul(const MemoryOperand& operand) const override;

    std::string idiv(const Register& operand) const override;
    std::string idiv(const MemoryOperand& operand) const override;
    std::string cqo() const override;

    std::string inc(const Register& operand) const override;
    std::string inc(const MemoryOperand& operand) const override;

    std::string dec(const Register& operand) const override;
    std::string dec(const MemoryOperand& operand) const override;

    std::string neg(const Register& operand) const override;
    std::vector<std::string> bswap(const Register& operand, int widthBytes) const override;

    std::string loadX87(const MemoryOperand& source) const override;
    std::string storeX87(const MemoryOperand& dest) const override;

    std::string loadByteSignExtend(const Register& address, const Register& dest) const override;
    std::string loadByteZeroExtend(const Register& address, const Register& dest) const override;
    std::string loadWordSignExtend(const Register& address, const Register& dest) const override;
    std::string loadWordZeroExtend(const Register& address, const Register& dest) const override;
    std::string loadDwordSignExtend(const Register& address, const Register& dest) const override;
    std::string storeByte(const Register& source, const Register& address) const override;
    std::string storeDword(const Register& source, const Register& address) const override;

protected:
    std::string preamblePrefix() const override;
    std::string globlDataLine(const std::string& name) const override;
    std::string dataSectionHeader() const override;
    std::string textSectionHeader() const override;
    std::string constantLine(const std::string& name, const std::string& escapedValue) const override;
    std::string dataObjectLines(const GlobalVariable& global) const override;
};

} // namespace codegen

#endif // INTEL_INSTRUCTIONSET_H_
