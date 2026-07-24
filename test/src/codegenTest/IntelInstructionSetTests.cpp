#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/IntelInstructionSet.h"
#include "codegen/MemoryOperand.h"
#include "codegen/Register.h"

namespace {

using namespace testing;
using namespace codegen;

// Production backend is Intel/NASM; keep these in lockstep with InstructionSet
// contracts that StackMachine and the linker rely on.
IntelInstructionSet instructions;

TEST(IntelInstructionSet, preambleUsesNasmSectionsAndDefaultRel) {
    std::string out = instructions.preamble({}, {}, {}, { "main" });
    EXPECT_THAT(out, HasSubstr("default rel\n"));
    EXPECT_THAT(out, HasSubstr("section .data\n"));
    EXPECT_THAT(out, HasSubstr("section .text\n"));
    EXPECT_THAT(out, HasSubstr("global main\n"));
    EXPECT_THAT(out, HasSubstr("extern printf\n"));
    EXPECT_THAT(out, HasSubstr("extern __trans_va_set_areas\n"));
}

TEST(IntelInstructionSet, labelAndCallPrefixReservedSymbols) {
    // NASM treats bare "strict" as a keyword; $strict is a safe symbol form.
    EXPECT_THAT(instructions.label("strict"), Eq("$strict:"));
    EXPECT_THAT(instructions.call("strict"), Eq("call $strict"));
    EXPECT_THAT(instructions.call("printf"), Eq("call $printf"));
}

TEST(IntelInstructionSet, emitsMovToMemoryWithOffset) {
    Register source { "rax" };
    Register memoryBase { "rbp" };
    EXPECT_THAT(instructions.mov(source, MemoryOperand::at(memoryBase, 16)),
            Eq("mov [rbp + 16], rax"));
}

TEST(IntelInstructionSet, emitsMovToMemoryWithoutOffset) {
    Register source { "rax" };
    Register memoryBase { "rsp" };
    EXPECT_THAT(instructions.mov(source, MemoryOperand::at(memoryBase, 0)),
            Eq("mov [rsp], rax"));
}

TEST(IntelInstructionSet, emitsLeaAndSub) {
    Register base { "rbp" };
    Register dest { "rdi" };
    EXPECT_THAT(instructions.lea(MemoryOperand::at(base, 24), dest),
            Eq("lea rdi, [rbp + 24]"));
    EXPECT_THAT(instructions.sub(dest, 32), Eq("sub rdi, 32"));
}

TEST(IntelInstructionSet, emitsLeaveRetAndPushPop) {
    Register reg { "rbx" };
    EXPECT_THAT(instructions.push(reg), Eq("push rbx"));
    EXPECT_THAT(instructions.pop(reg), Eq("pop rbx"));
    EXPECT_THAT(instructions.leave(), Eq("leave"));
    EXPECT_THAT(instructions.ret(), Eq("ret"));
}

TEST(IntelInstructionSet, xorSameRegisterClears) {
    Register rax { "rax" };
    EXPECT_THAT(instructions.xor_(rax, rax), Eq("xor rax, rax"));
}

TEST(IntelInstructionSet, shrIsLogicalAndSarIsArithmetic) {
    Register rax { "rax" };
    EXPECT_THAT(instructions.shr(rax), Eq("shr rax, cl"));
    EXPECT_THAT(instructions.sar(rax), Eq("sar rax, cl"));
}

} // namespace
