#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/IntelInstructionSet.h"
#include "codegen/MemoryOperand.h"
#include "codegen/Register.h"

namespace {

using namespace testing;
using namespace codegen;

IntelInstructionSet instructions;

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

} // namespace
