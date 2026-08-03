#ifndef ATANDTINSTRUCTIONSET_H_
#define ATANDTINSTRUCTIONSET_H_

#include <string>

#include "InstructionSet.h"

namespace codegen {

class ATandTInstructionSet: public InstructionSet {
public:
    virtual ~ATandTInstructionSet();

    std::string globl(const std::string& name) const override;
    std::string externDirective(const std::string& name) const override;
    std::string callPlt(std::string procedureName) const override;
    std::string callIndirect(const Register& target) const override;
    std::string loadGot(std::string symbolName, const Register& target) const override;

    std::string push(const Register& reg) const override;
    std::string pop(const Register& reg) const override;

    std::string add(const Register& reg, int constant, GprWidth width) const override;
    std::string sub(const Register& reg, int constant, GprWidth width) const override;

    std::string lea(const MemoryOperand& source, const Register& target) const override;

    std::string not_(const Register& reg, GprWidth width) const override;

    std::string mov(const Register& from, const MemoryOperand& destination) const override;
    std::string mov(const Register& from, const Register& to) const override;
    std::string mov(const MemoryOperand& source, const Register& to) const override;
    std::string mov(std::string constant, const MemoryOperand& destination) const override;
    std::string mov(std::string constant, const Register& to) const override;

    std::string sseGprXmm(SseGprXmmDir dir, SseWidth width, const Register& gpr,
            int xmmIndex) const override;
    std::string sseXmmToMem(int xmmIndex, const MemoryOperand& dest) const override;
    std::string sseCvtIntToXmm(const Register& gpr, int xmmIndex, SseWidth dest) const override;
    std::string sseCvtTruncToGpr(int xmmIndex, const Register& gpr, SseWidth src) const override;
    std::string sseCvtFloat(SseWidth from, SseWidth to, int srcXmm, int dstXmm) const override;
    std::string sseBin(SseBin op, SseWidth width, int dstXmm, int srcXmm) const override;
    std::string sseUcomi(SseWidth width, int leftXmm, int rightXmm) const override;

    std::string cqo() const override;

    std::string shrImm(const Register& reg, int amount) const override;


    std::string cmp(const Register& leftArgument, const Register& rightArgument,
            GprWidth width) const override;
    std::string cmp(const Register& argument, int constant, GprWidth width) const override;

    std::string xor_(const Register& operand, const Register& result, GprWidth width) const override;

    std::string or_(const Register& operand, const Register& result, GprWidth width) const override;

    std::string and_(const Register& operand, const Register& result, GprWidth width) const override;

    std::string shl(const Register& result, GprWidth width) const override;
    std::string shr(const Register& result, GprWidth width) const override;
    std::string sar(const Register& result, GprWidth width) const override;
    std::string shld(const Register& source, const Register& dest) const override;
    std::string shrd(const Register& source, const Register& dest) const override;

    std::string add(const Register& operand, const Register& result, GprWidth width) const override;
    std::string adc(const Register& operand, const Register& result) const override;

    std::string sub(const Register& operand, const Register& result, GprWidth width) const override;
    std::string sbb(const Register& operand, const Register& result) const override;

    std::string imul(const Register& operand, GprWidth width) const override;

    std::string idiv(const Register& operand, GprWidth width) const override;
    std::string div(const Register& operand, GprWidth width) const override;
    std::string cdq() const override;

    using InstructionSet::inc;
    std::string inc(const Register& operand, GprWidth width) const override;

    using InstructionSet::dec;
    std::string dec(const Register& operand, GprWidth width) const override;

    std::string neg(const Register& operand, GprWidth width) const override;
    std::string ctz(const Register& operand, int widthBytes) const override;

    std::string loadX87(const MemoryOperand& source, int sizeBytes = 16) const override;
    std::string storeX87(const MemoryOperand& dest, int sizeBytes = 16) const override;
    std::string fild(const MemoryOperand& source, int sizeBytes) const override;
    std::string fisttp(const MemoryOperand& dest, int sizeBytes) const override;
    std::string faddp() const override;
    std::string fsubp() const override;
    std::string fmulp() const override;
    std::string fdivp() const override;
    std::string fucomip() const override;
    std::string fstpSt0() const override;

protected:
    std::string dataSectionHeader() const override;
    std::string textSectionHeader() const override;
    std::string constantLine(const std::string& name, const std::string& escapedValue) const override;
    std::string dataObjectLines(const GlobalVariable& global) const override;

    std::string loadW(const MemoryOperand& source, const Register& dest, AccessWidth width,
            bool isSigned) const override;
    std::string storeW(const Register& source, const MemoryOperand& dest, AccessWidth width) const override;
    std::string extendW(const Register& reg, AccessWidth width, bool isSigned) const override;
    std::string storeImmW(const MemoryOperand& dest, long long imm, AccessWidth width) const override;
    std::vector<std::string> bswapW(const Register& operand, AccessWidth width) const override;
    std::string incW(const MemoryOperand& operand, AccessWidth width) const override;
    std::string decW(const MemoryOperand& operand, AccessWidth width) const override;
};

} // namespace codegen

#endif // ATANDTINSTRUCTIONSET_H_
