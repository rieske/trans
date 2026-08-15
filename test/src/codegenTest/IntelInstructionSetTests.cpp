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

TEST(IntelInstructionSet, emitsNarrowExtendsFromMemoryOperand) {
    Register dest { "rbx" };
    Register rbp { "rbp" };
    EXPECT_THAT(instructions.loadByteSignExtend(MemoryOperand::at(rbp, -8), dest),
            Eq("movsx rbx, byte [rbp + -8]"));
    EXPECT_THAT(instructions.loadByteZeroExtend(MemoryOperand::at(rbp, -8), dest),
            Eq("movzx rbx, byte [rbp + -8]"));
    EXPECT_THAT(instructions.loadWordSignExtend(MemoryOperand::at(rbp, -16), dest),
            Eq("movsx rbx, word [rbp + -16]"));
    EXPECT_THAT(instructions.loadDwordSignExtend(MemoryOperand::at(rbp, -32), dest),
            Eq("movsxd rbx, dword [rbp + -32]"));
}

TEST(IntelInstructionSet, emitsNarrowStoresToMemoryOperand) {
    Register src { "rax" };
    Register rbp { "rbp" };
    EXPECT_THAT(instructions.storeByte(src, MemoryOperand::at(rbp, -8)), Eq("mov byte [rbp + -8], al"));
    EXPECT_THAT(instructions.storeWord(src, MemoryOperand::at(rbp, -16)), Eq("mov word [rbp + -16], ax"));
    EXPECT_THAT(instructions.storeByte(src, MemoryOperand::global("flag")), Eq("mov byte [rel $flag], al"));
}

TEST(IntelInstructionSet, emitsIntegerOpsOnMemoryOperand) {
    Register rax { "rax" };
    Register rbp { "rbp" };
    const MemoryOperand slot = MemoryOperand::at(rbp, -8);
    EXPECT_THAT(instructions.cmp(rax, slot), Eq("cmp rax, qword [rbp + -8]"));
    EXPECT_THAT(instructions.cmp(slot, rax), Eq("cmp qword [rbp + -8], rax"));
    EXPECT_THAT(instructions.cmp(slot, 0), Eq("cmp qword [rbp + -8], 0"));
    EXPECT_THAT(instructions.cmp(slot, 0, 4), Eq("cmp dword [rbp + -8], 0"));
    EXPECT_THAT(instructions.add(slot, rax), Eq("add rax, [rbp + -8]"));
    EXPECT_THAT(instructions.add(slot, rax, 4), Eq("add eax, [rbp + -8]"));
    EXPECT_THAT(instructions.sub(slot, rax), Eq("sub rax, [rbp + -8]"));
    EXPECT_THAT(instructions.and_(slot, rax), Eq("and rax, [rbp + -8]"));
    EXPECT_THAT(instructions.or_(slot, rax), Eq("or rax, [rbp + -8]"));
    EXPECT_THAT(instructions.xor_(slot, rax), Eq("xor rax, [rbp + -8]"));
    EXPECT_THAT(instructions.imul(slot), Eq("imul qword [rbp + -8]"));
    EXPECT_THAT(instructions.imul(slot, 4), Eq("imul dword [rbp + -8]"));
    EXPECT_THAT(instructions.idiv(slot), Eq("idiv qword [rbp + -8]"));
    EXPECT_THAT(instructions.idiv(slot, 4), Eq("idiv dword [rbp + -8]"));
    EXPECT_THAT(instructions.div(slot), Eq("div qword [rbp + -8]"));
    EXPECT_THAT(instructions.div(slot, 4), Eq("div dword [rbp + -8]"));
    EXPECT_THAT(instructions.inc(slot), Eq("inc qword [rbp + -8]"));
    EXPECT_THAT(instructions.inc(slot, 4), Eq("inc dword [rbp + -8]"));
    EXPECT_THAT(instructions.dec(slot), Eq("dec qword [rbp + -8]"));
    EXPECT_THAT(instructions.dec(slot, 4), Eq("dec dword [rbp + -8]"));
}

TEST(IntelInstructionSet, emitsDwordIntegerOps) {
    Register src { "rsi" };
    Register dst { "rdi" };
    EXPECT_THAT(instructions.lshr(dst, 8), Eq("shr rdi, cl"));
    EXPECT_THAT(instructions.lshr(dst, 4), Eq("shr edi, cl"));
    EXPECT_THAT(instructions.shl(dst, 4), Eq("shl edi, cl"));
    EXPECT_THAT(instructions.shr(dst, 4), Eq("sar edi, cl"));
    EXPECT_THAT(instructions.add(src, dst, 4), Eq("add edi, esi"));
    EXPECT_THAT(instructions.cmp(dst, 1, 4), Eq("cmp edi, 1"));
    EXPECT_THAT(instructions.cdq(), Eq("cdq"));
}

} // namespace
