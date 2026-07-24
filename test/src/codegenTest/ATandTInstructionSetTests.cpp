#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/ATandTInstructionSet.h"
#include "codegen/MemoryOperand.h"
#include "codegen/Register.h"

#include <memory>
#include <stdexcept>

namespace {

using namespace testing;
using namespace codegen;

ATandTInstructionSet instructions;

TEST(ATandTInstructionSet, emitsPreamble) {
    EXPECT_THAT(instructions.preamble({}), Eq(".extern scanf\n"
            ".extern printf\n\n"
            ".data\n"
            "sfmt: .string \"%d\"\n"
            "fmt: .string \"%d\n\"\n\n"
            ".text\n"
            ".globl main\n\n"));
}

TEST(ATandTInstructionSet, emitsMovToMemoryWithOffset) {
    Register source { "src" };
    Register memoryBase { "memBase" };
    EXPECT_THAT(instructions.mov(source, MemoryOperand::at(memoryBase, 42)), Eq("movq %src, -42(%memBase)"));
}

TEST(ATandTInstructionSet, emitsMovToMemoryWithoutOffset) {
    Register source { "src" };
    Register memoryBase { "memBase" };
    EXPECT_THAT(instructions.mov(source, MemoryOperand::at(memoryBase, 0)), Eq("movq %src, (%memBase)"));
}

TEST(ATandTInstructionSet, emitsQuadSubtract) {
    Register reg { "reg" };
    EXPECT_THAT(instructions.sub(reg, 42), Eq("subq $42, %reg"));
}

// Implemented ops beyond mov-to-memory / sub-imm.
TEST(ATandTInstructionSet, emitsCallPushPopMovRegAndLabel) {
    Register rax { "rax" };
    Register rbp { "rbp" };
    Register rbx { "rbx" };
    MemoryOperand slot = MemoryOperand::at(rbp, 16);

    EXPECT_THAT(instructions.call("printf"), Eq("call printf"));
    EXPECT_THAT(instructions.push(rbx), Eq("pushq %rbx"));
    EXPECT_THAT(instructions.pop(rbx), Eq("popq %rbx"));
    EXPECT_THAT(instructions.add(rax, 8), Eq("addq $8, %rax"));
    // Identity mov is a no-op in this backend.
    EXPECT_THAT(instructions.mov(rax, rax), Eq(""));
    Register rcx { "rcx" };
    EXPECT_THAT(instructions.mov(rax, rcx), Eq("movq %rax, %rcx"));
    EXPECT_THAT(instructions.mov(slot, rax), Eq("movq -16(%rbp), %rax"));
    EXPECT_THAT(instructions.label("L1"), Eq("L1:"));
    EXPECT_THAT(instructions.leave(), Eq("leave"));
    EXPECT_THAT(instructions.ret(), Eq("ret"));
}

// Unimplemented AT&T stubs still throw — exercise those lines for coverage.
TEST(ATandTInstructionSet, unimplementedOpsThrow) {
    Register rax { "rax" };
    Register rbp { "rbp" };
    MemoryOperand slot = MemoryOperand::at(rbp, 8);
    EXPECT_THROW(instructions.lea(slot, rax), std::runtime_error);
    EXPECT_THROW(instructions.not_(rax), std::runtime_error);
    EXPECT_THROW(instructions.mov("1", rax), std::runtime_error);
    EXPECT_THROW(instructions.mov("1", slot), std::runtime_error);
    EXPECT_THROW(instructions.cmp(rax, slot), std::runtime_error);
    EXPECT_THROW(instructions.cmp(rax, rax), std::runtime_error);
    EXPECT_THROW(instructions.cmp(slot, rax), std::runtime_error);
    EXPECT_THROW(instructions.cmp(rax, 0), std::runtime_error);
    EXPECT_THROW(instructions.cmp(slot, 0), std::runtime_error);
    EXPECT_THROW(instructions.jmp("L"), std::runtime_error);
    EXPECT_THROW(instructions.je("L"), std::runtime_error);
    EXPECT_THROW(instructions.jne("L"), std::runtime_error);
    EXPECT_THROW(instructions.xor_(slot, rax), std::runtime_error);
    EXPECT_THROW(instructions.shr(rax), std::runtime_error);
    EXPECT_THROW(instructions.sar(rax), std::runtime_error);
}

}
