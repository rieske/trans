#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/GlobalVariable.h"
#include "codegen/IntelInstructionSet.h"
#include "codegen/MemoryOperand.h"
#include "codegen/Register.h"
#include "codegen/Sse.h"
#include "symbols/GlobalInitializer.h"

#include <stdexcept>

namespace {

using namespace testing;
using namespace codegen;

// Production backend is Intel/NASM; keep these in lockstep with InstructionSet
// contracts that StackMachine and the linker rely on.
IntelInstructionSet instructions;

TEST(IntelInstructionSet, preambleUsesNasmSectionsAndDefaultRel) {
    std::string out = instructions.preamble({});
    EXPECT_THAT(out, HasSubstr("default rel\n"));
    EXPECT_THAT(out, HasSubstr("section .data\n"));
    EXPECT_THAT(out, HasSubstr("section .text\n"));
    EXPECT_THAT(out, Not(HasSubstr("extern printf\n")));
    EXPECT_THAT(out, Not(HasSubstr("__trans_va_")));
}

TEST(IntelInstructionSet, labelAndCallPrefixReservedSymbols) {
    // NASM treats bare "strict" as a keyword; $strict is a safe symbol form.
    EXPECT_THAT(instructions.label("strict"), Eq("$strict:"));
    EXPECT_THAT(instructions.call("strict"), Eq("call $strict"));
    EXPECT_THAT(instructions.call("printf"), Eq("call $printf"));
}

TEST(IntelInstructionSet, asmSymbolPrefixesSourceNames) {
    EXPECT_THAT(instructions.asmSymbol("abs"), Eq("$abs"));
    EXPECT_THAT(instructions.asmSymbol("mov"), Eq("$mov"));
    EXPECT_THAT(instructions.asmSymbol("rax"), Eq("$rax"));
}

TEST(IntelInstructionSet, asmSymbolLeavesAlreadyEscapedNames) {
    EXPECT_THAT(instructions.asmSymbol("$s1x"), Eq("$s1x"));
    EXPECT_THAT(instructions.asmSymbol("$t0"), Eq("$t0"));
}

TEST(IntelInstructionSet, asmSymbolLeavesEmptyName) {
    EXPECT_THAT(instructions.asmSymbol(""), Eq(""));
}

TEST(IntelInstructionSet, globlAndExternUseRawElfNames) {
    EXPECT_THAT(instructions.globl("abs"), Eq("global abs"));
    EXPECT_THAT(instructions.externDirective("abs"), Eq("extern abs"));
}

TEST(IntelInstructionSet, callPltEscapesLibcName) {
    EXPECT_THAT(instructions.callPlt("abs"), Eq("call $abs wrt ..plt"));
}

TEST(IntelInstructionSet, loadGotEscapesLibcName) {
    Register target { "rax" };
    EXPECT_THAT(instructions.loadGot("abs", target),
            Eq("mov rax, [rel $abs wrt ..got]"));
}

TEST(IntelInstructionSet, leaGlobalEscapesLabel) {
    Register dest { "rbx" };
    EXPECT_THAT(instructions.lea(MemoryOperand::global("neg"), dest),
            Eq("lea rbx, [rel $neg]"));
}

TEST(IntelInstructionSet, labelEscapesReservedWord) {
    EXPECT_THAT(instructions.label("abs"), Eq("$abs:"));
}

TEST(IntelInstructionSet, dataObjectLinesPrefixReservedSymbols) {
    GlobalVariable strict;
    strict.name = "strict";
    strict.sizeInBytes = 8;
    strict.initializer = symbols::ConstantInit { 3 };
    strict.emission = ObjectEmission::DefineInternal;
    EXPECT_THAT(instructions.preamble({}, { strict }, {}),
            AllOf(HasSubstr("\t$strict dq 3\n"), Not(HasSubstr("\tstrict dq"))));
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

TEST(IntelInstructionSet, emitsX87LoadStore) {
    Register base { "rbp" };
    EXPECT_THAT(instructions.loadX87(MemoryOperand::at(base, -16)), Eq("fld tword [rbp + -16]"));
    EXPECT_THAT(instructions.storeX87(MemoryOperand::at(base, -16)), Eq("fstp tword [rbp + -16]"));
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

TEST(IntelInstructionSet, emitsDoubleShift) {
    Register src { "rsi" };
    Register dst { "rdi" };
    EXPECT_THAT(instructions.shld(src, dst), Eq("shld rdi, rsi, cl"));
    EXPECT_THAT(instructions.shrd(src, dst), Eq("shrd rdi, rsi, cl"));
}

TEST(IntelInstructionSet, sseBinUsesSharedMnemonic) {
    EXPECT_THAT(instructions.sseBin(SseBin::Add, SseWidth::F32, 0, 1), Eq("addss xmm0, xmm1"));
    EXPECT_THAT(instructions.sseBin(SseBin::Add, SseWidth::F64, 0, 1), Eq("addsd xmm0, xmm1"));
}

TEST(IntelInstructionSet, sseCvtFloatRejectsIdenticalWidths) {
    EXPECT_THAT(instructions.sseCvtFloat(SseWidth::F32, SseWidth::F64, 1, 0),
            Eq("cvtss2sd xmm0, xmm1"));
    EXPECT_THAT(instructions.sseCvtFloat(SseWidth::F64, SseWidth::F32, 1, 0),
            Eq("cvtsd2ss xmm0, xmm1"));
    EXPECT_THROW(instructions.sseCvtFloat(SseWidth::F32, SseWidth::F32, 0, 0), std::runtime_error);
}

} // namespace
