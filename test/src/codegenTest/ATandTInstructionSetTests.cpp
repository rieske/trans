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

}
