#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/ATandTInstructionSet.h"
#include "codegen/GlobalVariable.h"
#include "symbols/GlobalInitializer.h"
#include "codegen/MemoryOperand.h"
#include "codegen/Register.h"
#include "codegen/Sse.h"

#include <memory>
#include <stdexcept>

namespace {

using namespace testing;
using namespace codegen;

ATandTInstructionSet instructions;

TEST(ATandTInstructionSet, emitsPreamble) {
    const std::string preamble = instructions.preamble({});
    EXPECT_THAT(preamble, Not(HasSubstr(".extern scanf\n")));
    EXPECT_THAT(preamble, Not(HasSubstr(".extern printf\n")));
    EXPECT_THAT(preamble, Not(HasSubstr(".extern malloc\n")));
    EXPECT_THAT(preamble, Not(HasSubstr("__trans_va_")));
    EXPECT_THAT(preamble, HasSubstr("\n.section .data\n"));
    EXPECT_THAT(preamble, HasSubstr("\n.section .text\n"));
}

TEST(ATandTInstructionSet, preambleSkipsExternalGlobalsAndEmitsData) {
    GlobalVariable ext;
    ext.name = "environ";
    ext.sizeInBytes = 8;
    ext.emission = ObjectEmission::Reference;

    GlobalVariable scalar;
    scalar.name = "g";
    scalar.sizeInBytes = 8;
    scalar.initializer = symbols::ConstantInit { 42 };
    scalar.emission = ObjectEmission::DefineExternal;

    const std::string preamble = instructions.preamble(
            { { "msg", "\"hi\"" } }, { ext, scalar }, { "environ" });
    EXPECT_THAT(preamble, HasSubstr(".extern environ\n"));
    EXPECT_THAT(preamble, Not(HasSubstr("environ:\n")));
    EXPECT_THAT(preamble, HasSubstr(".balign 8\n"));
    EXPECT_THAT(preamble, HasSubstr("msg:\n\t.byte 104, 105, 0\n"));
    EXPECT_THAT(preamble, HasSubstr("g:\n\t.quad 42\n"));
    EXPECT_THAT(preamble, HasSubstr(".globl g\n"));
    EXPECT_THAT(preamble, Not(HasSubstr(".globl msg\n")));
}

TEST(ATandTInstructionSet, preambleAlignsDataObject) {
    GlobalVariable gv;
    gv.name = "x";
    gv.sizeInBytes = 8;
    gv.alignBytes = 8;
    gv.emission = ObjectEmission::DefineInternal;
    EXPECT_THAT(instructions.preamble({}, { gv }), Eq("\n.section .data\n"
            "\t.align 8\n"
            "x:\n\t.quad 0\n"
            "\n.section .text\n\n"));
}

TEST(ATandTInstructionSet, preambleEmitsOnlyRequestedExterns) {
    const std::string preamble = instructions.preamble({}, {}, { "printf", "strtod" });
    EXPECT_THAT(preamble, HasSubstr(".extern printf\n"));
    EXPECT_THAT(preamble, HasSubstr(".extern strtod\n"));
}

TEST(ATandTInstructionSet, dataObjectLinesKeepsDialectNeutralStaticHome) {
    GlobalVariable localStatic;
    localStatic.name = "L$st3_n";
    localStatic.sizeInBytes = 8;
    localStatic.initializer = symbols::ConstantInit { 0 };
    localStatic.emission = ObjectEmission::DefineInternal;
    const std::string preamble = instructions.preamble({}, { localStatic }, {});
    EXPECT_THAT(preamble, HasSubstr("L$st3_n:\n\t.quad 0\n"));
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
    MemoryOperand mem = MemoryOperand::at(addr, 0);
    EXPECT_THAT(instructions.load(mem, dest, 1, false), Eq("movzbq (%rax), %rbx"));
    EXPECT_THAT(instructions.load(mem, dest, 2, true), Eq("movswq (%rax), %rbx"));
    EXPECT_THAT(instructions.load(mem, dest, 2, false), Eq("movzwq (%rax), %rbx"));
    EXPECT_THAT(instructions.load(mem, dest, 4, true), Eq("movslq (%rax), %rbx"));
}

TEST(ATandTInstructionSet, emitsQuadSubtract) {
    Register reg { "rdi" };
    EXPECT_THAT(instructions.sub(reg, 42, GprWidth::W64), Eq("subq $42, %rdi"));
    EXPECT_THAT(instructions.add(reg, 1, GprWidth::W32), Eq("addl $1, %edi"));
}

TEST(ATandTInstructionSet, emitsCqo) {
    EXPECT_THAT(instructions.cqo(), Eq("cqto"));
}

TEST(ATandTInstructionSet, memoryIncDecUsesObjectSize) {
    Register base { "rbp" };
    MemoryOperand mem = MemoryOperand::at(base, -4);
    EXPECT_THAT(instructions.inc(mem, 1), Eq("incb -4(%rbp)"));
    EXPECT_THAT(instructions.inc(mem, 2), Eq("incw -4(%rbp)"));
    EXPECT_THAT(instructions.inc(mem, 4), Eq("incl -4(%rbp)"));
    EXPECT_THAT(instructions.inc(mem, 8), Eq("incq -4(%rbp)"));
    EXPECT_THAT(instructions.dec(mem, 1), Eq("decb -4(%rbp)"));
}

TEST(ATandTInstructionSet, emitsUnsignedDiv) {
    Register dst { "rdi" };
    EXPECT_THAT(instructions.div(dst, GprWidth::W64), Eq("divq %rdi"));
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

TEST(ATandTInstructionSet, emitsDoubleShift) {
    Register src { "rsi" };
    Register dst { "rdi" };
    EXPECT_THAT(instructions.shld(src, dst), Eq("shldq %cl, %rsi, %rdi"));
    EXPECT_THAT(instructions.shrd(src, dst), Eq("shrdq %cl, %rsi, %rdi"));
}

TEST(ATandTInstructionSet, emitsDwordIntegerOps) {
    Register src { "rsi" };
    Register dst { "rdi" };
    EXPECT_THAT(instructions.shr(dst, GprWidth::W64), Eq("shrq %cl, %rdi"));
    EXPECT_THAT(instructions.shr(dst, GprWidth::W32), Eq("shrl %cl, %edi"));
    EXPECT_THAT(instructions.shl(dst, GprWidth::W32), Eq("shll %cl, %edi"));
    EXPECT_THAT(instructions.sar(dst, GprWidth::W32), Eq("sarl %cl, %edi"));
    EXPECT_THAT(instructions.add(src, dst, GprWidth::W32), Eq("addl %esi, %edi"));
    EXPECT_THAT(instructions.cmp(dst, 1, GprWidth::W32), Eq("cmpl $1, %edi"));
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

TEST(ATandTInstructionSet, sseBinUsesSharedMnemonic) {
    EXPECT_THAT(instructions.sseBin(SseBin::Add, SseWidth::F32, 0, 1), Eq("addss %xmm1, %xmm0"));
    EXPECT_THAT(instructions.sseBin(SseBin::Add, SseWidth::F64, 0, 1), Eq("addsd %xmm1, %xmm0"));
}

TEST(ATandTInstructionSet, sseCvtFloatRejectsIdenticalWidths) {
    EXPECT_THROW(instructions.sseCvtFloat(SseWidth::F64, SseWidth::F64, 0, 0), std::runtime_error);
}

}
