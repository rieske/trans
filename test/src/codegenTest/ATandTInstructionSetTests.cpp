#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/ATandTInstructionSet.h"
#include "codegen/GlobalVariable.h"
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
    // Empty external/defined lists: still emit default libc + va_tls externs and main
    // (parity with IntelInstructionSet defaults).
    const std::string preamble = instructions.preamble({}, {}, {}, {});
    EXPECT_THAT(preamble, HasSubstr(".extern scanf\n"));
    EXPECT_THAT(preamble, HasSubstr(".extern printf\n"));
    EXPECT_THAT(preamble, HasSubstr(".extern malloc\n"));
    EXPECT_THAT(preamble, HasSubstr(".extern __trans_va_set_areas\n"));
    EXPECT_THAT(preamble, HasSubstr("\n.section .data\n"));
    EXPECT_THAT(preamble, HasSubstr("\n.section .text\n"));
    EXPECT_THAT(preamble, HasSubstr(".globl main\n"));
}

TEST(ATandTInstructionSet, preambleSkipsExternalGlobalsAndEmitsStringBytes) {
    GlobalVariable ext;
    ext.name = "environ";
    ext.sizeInBytes = 8;
    ext.initializerLiteral = "0";
    ext.isExternal = true;

    GlobalVariable str;
    str.name = "msg";
    str.sizeInBytes = 3;
    str.stringInitializer = "\"hi\"";
    str.initializerLiteral = "0";

    GlobalVariable scalar;
    scalar.name = "g";
    scalar.sizeInBytes = 8;
    scalar.initializerLiteral = "42";

    const std::string preamble = instructions.preamble({}, { ext, str, scalar }, {}, { "main" });
    EXPECT_THAT(preamble, HasSubstr(".extern environ\n"));
    EXPECT_THAT(preamble, Not(HasSubstr("environ:\n")));
    EXPECT_THAT(preamble, HasSubstr(".balign 8\n"));
    EXPECT_THAT(preamble, HasSubstr("msg:\n\t.byte 104, 105, 0\n"));
    EXPECT_THAT(preamble, HasSubstr("g:\n\t.quad 42\n"));
    EXPECT_THAT(preamble, HasSubstr(".globl g\n"));
    EXPECT_THAT(preamble, HasSubstr(".globl msg\n"));
}

TEST(ATandTInstructionSet, preambleEmitsOnlyRequestedExterns) {
    const std::string preamble = instructions.preamble({}, {}, { "printf", "strtod" }, {});
    EXPECT_THAT(preamble, HasSubstr(".extern printf\n"));
    EXPECT_THAT(preamble, HasSubstr(".extern strtod\n"));
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

TEST(ATandTInstructionSet, emitsQuadSubtract) {
    Register reg { "reg" };
    EXPECT_THAT(instructions.sub(reg, 42), Eq("subq $42, %reg"));
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

TEST(ATandTInstructionSet, sseBinUsesSharedMnemonic) {
    EXPECT_THAT(instructions.sseBin(SseBin::Add, SseWidth::F32, 0, 1), Eq("addss %xmm1, %xmm0"));
    EXPECT_THAT(instructions.sseBin(SseBin::Add, SseWidth::F64, 0, 1), Eq("addsd %xmm1, %xmm0"));
}

TEST(ATandTInstructionSet, sseCvtFloatRejectsIdenticalWidths) {
    EXPECT_THROW(instructions.sseCvtFloat(SseWidth::F64, SseWidth::F64, 0, 0), std::runtime_error);
}

}
