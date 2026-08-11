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
    EXPECT_THAT(instructions.loadByteZeroExtend(addr, dest), Eq("movzbq (%rax), %rbx"));
    EXPECT_THAT(instructions.loadWordSignExtend(addr, dest), Eq("movswq (%rax), %rbx"));
    EXPECT_THAT(instructions.loadWordZeroExtend(addr, dest), Eq("movzwq (%rax), %rbx"));
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
    EXPECT_THAT(instructions.lshr(dst), Eq("shrq %cl, %rdi"));
}

TEST(ATandTInstructionSet, emitsLoadGot) {
    Register target { "rax" };
    EXPECT_THAT(instructions.loadGot("printf", target),
            Eq("movq printf@GOTPCREL(%rip), %rax"));
}

TEST(ATandTInstructionSet, emitsCallPlt) {
    EXPECT_THAT(instructions.callPlt("printf"), Eq("call printf@plt"));
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

}
