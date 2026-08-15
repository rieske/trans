#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/ATandTInstructionSet.h"
#include "codegen/MemoryOperand.h"
#include "codegen/Register.h"

#include <memory>

namespace {

using namespace testing;
using namespace codegen;

ATandTInstructionSet instructions;

TEST(ATandTInstructionSet, emitsPreamble) {
    EXPECT_THAT(instructions.preamble({}), Eq("\n.section .data\n"
            "\n.section .text\n\n"));
}

TEST(ATandTInstructionSet, preambleEmitsOnlyRequestedExterns) {
    EXPECT_THAT(instructions.preamble({}, {}, { "printf", "strtod" }),
            Eq(".extern printf\n"
                    ".extern strtod\n"
                    "\n.section .data\n"
                    "\n.section .text\n\n"));
}

TEST(ATandTInstructionSet, emitsMovToMemoryWithOffset) {
    Register source { "src" };
    Register memoryBase { "memBase" };
    EXPECT_THAT(instructions.mov(source, MemoryOperand::at(memoryBase, 42)), Eq("movq %src, 42(%memBase)"));
}

TEST(ATandTInstructionSet, emitsMovToMemoryWithoutOffset) {
    Register source { "src" };
    Register memoryBase { "memBase" };
    EXPECT_THAT(instructions.mov(source, MemoryOperand::at(memoryBase, 0)), Eq("movq %src, (%memBase)"));
}

TEST(ATandTInstructionSet, emitsNarrowExtends) {
    Register addr { "rax" };
    Register dest { "rbx" };
    EXPECT_THAT(instructions.loadByteZeroExtend(MemoryOperand::at(addr, 0), dest),
            Eq("movzbq (%rax), %rbx"));
    EXPECT_THAT(instructions.loadWordSignExtend(MemoryOperand::at(addr, 0), dest),
            Eq("movswq (%rax), %rbx"));
    EXPECT_THAT(instructions.loadWordZeroExtend(MemoryOperand::at(addr, 0), dest),
            Eq("movzwq (%rax), %rbx"));
}

TEST(ATandTInstructionSet, emitsNarrowExtendsFromMemoryOperand) {
    Register dest { "rbx" };
    Register rbp { "rbp" };
    EXPECT_THAT(instructions.loadByteSignExtend(MemoryOperand::at(rbp, -8), dest),
            Eq("movsbq -8(%rbp), %rbx"));
    EXPECT_THAT(instructions.loadByteZeroExtend(MemoryOperand::at(rbp, -8), dest),
            Eq("movzbq -8(%rbp), %rbx"));
    EXPECT_THAT(instructions.loadWordSignExtend(MemoryOperand::at(rbp, -16), dest),
            Eq("movswq -16(%rbp), %rbx"));
    EXPECT_THAT(instructions.loadDwordSignExtend(MemoryOperand::at(rbp, -32), dest),
            Eq("movslq -32(%rbp), %rbx"));
}

TEST(ATandTInstructionSet, emitsNarrowStoresToMemoryOperand) {
    Register src { "rax" };
    Register rbp { "rbp" };
    EXPECT_THAT(instructions.storeByte(src, MemoryOperand::at(rbp, -8)), Eq("movb %al, -8(%rbp)"));
    EXPECT_THAT(instructions.storeWord(src, MemoryOperand::at(rbp, -16)), Eq("movw %ax, -16(%rbp)"));
    EXPECT_THAT(instructions.storeByte(src, MemoryOperand::global("flag")), Eq("movb %al, flag(%rip)"));
}

TEST(ATandTInstructionSet, emitsIntegerOpsOnMemoryOperand) {
    Register rax { "rax" };
    Register rbp { "rbp" };
    const MemoryOperand slot = MemoryOperand::at(rbp, -8);
    EXPECT_THAT(instructions.cmp(rax, slot), Eq("cmpq -8(%rbp), %rax"));
    EXPECT_THAT(instructions.cmp(slot, rax), Eq("cmpq %rax, -8(%rbp)"));
    EXPECT_THAT(instructions.cmp(slot, 0), Eq("cmpq $0, -8(%rbp)"));
    EXPECT_THAT(instructions.cmp(slot, 0, 4), Eq("cmpl $0, -8(%rbp)"));
    EXPECT_THAT(instructions.add(slot, rax), Eq("addq -8(%rbp), %rax"));
    EXPECT_THAT(instructions.add(slot, rax, 4), Eq("addl -8(%rbp), %eax"));
    EXPECT_THAT(instructions.sub(slot, rax), Eq("subq -8(%rbp), %rax"));
    EXPECT_THAT(instructions.and_(slot, rax), Eq("andq -8(%rbp), %rax"));
    EXPECT_THAT(instructions.or_(slot, rax), Eq("orq -8(%rbp), %rax"));
    EXPECT_THAT(instructions.xor_(slot, rax), Eq("xorq -8(%rbp), %rax"));
    EXPECT_THAT(instructions.imul(slot), Eq("imulq -8(%rbp)"));
    EXPECT_THAT(instructions.imul(slot, 4), Eq("imull -8(%rbp)"));
    EXPECT_THAT(instructions.idiv(slot), Eq("idivq -8(%rbp)"));
    EXPECT_THAT(instructions.idiv(slot, 4), Eq("idivl -8(%rbp)"));
    EXPECT_THAT(instructions.div(slot), Eq("divq -8(%rbp)"));
    EXPECT_THAT(instructions.div(slot, 4), Eq("divl -8(%rbp)"));
    EXPECT_THAT(instructions.inc(slot), Eq("incq -8(%rbp)"));
    EXPECT_THAT(instructions.inc(slot, 4), Eq("incl -8(%rbp)"));
    EXPECT_THAT(instructions.dec(slot), Eq("decq -8(%rbp)"));
    EXPECT_THAT(instructions.dec(slot, 4), Eq("decl -8(%rbp)"));
}

TEST(ATandTInstructionSet, emitsQuadSubtract) {
    Register reg { "reg" };
    EXPECT_THAT(instructions.sub(reg, 42), Eq("subq $42, %reg"));
}

TEST(ATandTInstructionSet, emitsCqo) {
    EXPECT_THAT(instructions.cqo(), Eq("cqto"));
}

TEST(ATandTInstructionSet, emitsUnsignedDiv) {
    Register dst { "rdi" };
    EXPECT_THAT(instructions.div(dst), Eq("divq %rdi"));
}

TEST(ATandTInstructionSet, emitsAdcSbbAndUnsignedJumps) {
    Register src { "rsi" };
    Register dst { "rdi" };
    EXPECT_THAT(instructions.adc(src, dst), Eq("adcq %rsi, %rdi"));
    EXPECT_THAT(instructions.sbb(src, dst), Eq("sbbq %rsi, %rdi"));
    EXPECT_THAT(instructions.ja("L"), Eq("ja L"));
    EXPECT_THAT(instructions.jb("L"), Eq("jb L"));
    EXPECT_THAT(instructions.jae("L"), Eq("jae L"));
    EXPECT_THAT(instructions.jbe("L"), Eq("jbe L"));
}

TEST(ATandTInstructionSet, emitsDoubleShiftAndLogicalShr) {
    Register src { "rsi" };
    Register dst { "rdi" };
    EXPECT_THAT(instructions.shld(src, dst), Eq("shldq %cl, %rsi, %rdi"));
    EXPECT_THAT(instructions.shrd(src, dst), Eq("shrdq %cl, %rsi, %rdi"));
    EXPECT_THAT(instructions.lshr(dst, 8), Eq("shrq %cl, %rdi"));
    EXPECT_THAT(instructions.lshr(dst, 4), Eq("shrl %cl, %edi"));
    EXPECT_THAT(instructions.shl(dst, 4), Eq("shll %cl, %edi"));
    EXPECT_THAT(instructions.shr(dst, 4), Eq("sarl %cl, %edi"));
    EXPECT_THAT(instructions.add(src, dst, 4), Eq("addl %esi, %edi"));
    EXPECT_THAT(instructions.cmp(dst, 1, 4), Eq("cmpl $1, %edi"));
    EXPECT_THAT(instructions.cdq(), Eq("cltd"));
}

TEST(ATandTInstructionSet, emitsLoadGot) {
    Register target { "rax" };
    EXPECT_THAT(instructions.loadGot("printf", target),
            Eq("movq printf@GOTPCREL(%rip), %rax"));
}

TEST(ATandTInstructionSet, emitsCallPlt) {
    EXPECT_THAT(instructions.callPlt("printf"), Eq("call printf@plt"));
}

TEST(ATandTInstructionSet, asmSymbolIsIdentity) {
    EXPECT_THAT(instructions.asmSymbol("abs"), Eq("abs"));
    EXPECT_THAT(instructions.asmSymbol("$s1x"), Eq("$s1x"));
}

TEST(ATandTInstructionSet, globlAndExternUseRawElfNames) {
    EXPECT_THAT(instructions.globl("abs"), Eq(".globl abs"));
    EXPECT_THAT(instructions.externDirective("abs"), Eq(".extern abs"));
}

TEST(ATandTInstructionSet, callNamedRegisterLikeLabelIsDirect) {
    EXPECT_THAT(instructions.call("r10"), Eq("call r10"));
}

TEST(ATandTInstructionSet, emitsCallIndirect) {
    Register target { "r10" };
    EXPECT_THAT(instructions.callIndirect(target), Eq("call *%r10"));
}

TEST(ATandTInstructionSet, emitsX87LoadStore) {
    Register base { "rbp" };
    EXPECT_THAT(instructions.loadX87(MemoryOperand::at(base, -16)), Eq("fldt -16(%rbp)"));
    EXPECT_THAT(instructions.storeX87(MemoryOperand::at(base, -16)), Eq("fstpt -16(%rbp)"));
    EXPECT_THAT(instructions.loadX87(MemoryOperand::at(base, -8), 8), Eq("fldl -8(%rbp)"));
    EXPECT_THAT(instructions.faddp(), Eq("faddp %st, %st(1)"));
    EXPECT_THAT(instructions.fsubp(), Eq("fsubrp %st, %st(1)"));
    EXPECT_THAT(instructions.fucomip(), Eq("fucomip %st(1), %st"));
}

TEST(ATandTInstructionSet, emitsBswapWidths) {
    Register rax { "rax" };
    EXPECT_THAT(instructions.bswap(rax, 2), ElementsAre("rolw $8, %ax", "andq $0xffff, %rax"));
    EXPECT_THAT(instructions.bswap(rax, 4), ElementsAre("bswap %eax"));
    EXPECT_THAT(instructions.bswap(rax, 8), ElementsAre("bswap %rax"));
}

TEST(ATandTInstructionSet, emitsCtzWidths) {
    Register rax { "rax" };
    EXPECT_THAT(instructions.ctz(rax, 4), Eq("bsfl %eax, %eax"));
    EXPECT_THAT(instructions.ctz(rax, 8), Eq("bsfq %rax, %rax"));
}

}
