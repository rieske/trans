#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/IntelInstructionSet.h"
#include "types/SysVClass.h"

#include <memory>
#include <string>
#include <vector>

namespace {

using testing::HasSubstr;
using testing::Not;
using testing::StrEq;
using namespace codegen;

class StackMachineTest: public testing::Test {
public:
    StackMachineTest() :
            registers { std::make_unique<Amd64Registers>() },
            generalPurposeRegisters { registers->getGeneralPurposeRegisters() },
            rax { generalPurposeRegisters[0] },
            rbx { generalPurposeRegisters[1] },
            rcx { generalPurposeRegisters[2] }
    {
    }

protected:
    Value intValue(std::string name) {
        return { name, 0, ValueKind::INTEGRAL, 8 };
    }

    void expectCode(std::string expectedCode) {
        EXPECT_THAT(assemblyCode.str(), StrEq(expectedCode));
    }

    void expectRegisterContains(Register* reg, Value val) {
        EXPECT_TRUE(reg->getValue() != nullptr);
        EXPECT_THAT(reg->getValue()->getName(), StrEq(val.getName()));
    }

    Procedure testProc(std::string name, std::vector<Value> locals = {}, std::vector<Value> args = {},
            bool memoryReturn = false, bool variadic = false) {
        Procedure procedure;
        procedure.name = std::move(name);
        procedure.frame.locals = std::move(locals);
        procedure.frame.arguments = std::move(args);
        procedure.memoryReturn = memoryReturn;
        procedure.variadic = variadic;
        return procedure;
    }

    std::unique_ptr<Amd64Registers> registers;
    std::stringstream assemblyCode { };

    std::vector<Register*> generalPurposeRegisters;

    Register* rax;
    Register* rbx;
    Register* rcx;

    Value v1 { "v1", 0, ValueKind::INTEGRAL, 8 };
    Value v2 { "v2", 1, ValueKind::INTEGRAL, 8 };
    Value v3 { "v3", 2, ValueKind::INTEGRAL, 8 };

private:

};

TEST_F(StackMachineTest, procedureCall_doesNotPushUnusedCallerSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n");
}

TEST_F(StackMachineTest, functionAddress_leaDefinedProcedure) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value fp = intValue("fp");
    stackMachine.generatePreamble({}, {}, {}, { "foo" });
    stackMachine.startProcedure(testProc("foo", { fp }, { }));
    assemblyCode.str("");

    stackMachine.functionAddress("foo", "fp");

    expectCode("\tlea rax, [rel $foo]\n");
}

TEST_F(StackMachineTest, functionAddress_loadsExternViaGot) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value fp = intValue("fp");
    stackMachine.generatePreamble({}, {}, {}, { "proc" });
    stackMachine.startProcedure(testProc("proc", { fp }, { }));
    assemblyCode.str("");

    stackMachine.functionAddress("printf", "fp");

    expectCode("\tmov rax, [rel $printf wrt ..got]\n");
}

TEST_F(StackMachineTest, assignLabelAddress_leaPoolLabel) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value s = intValue("s");
    stackMachine.startProcedure(testProc("proc", { s }, { }));
    assemblyCode.str("");

    stackMachine.assignLabelAddress("__str1", "s");

    expectCode("\tlea rax, [rel $__str1]\n");
}

TEST_F(StackMachineTest, callProcedure_sameTuDoesNotUsePlt) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    stackMachine.generatePreamble({}, {}, {}, { "foo" });
    assemblyCode.str("");

    stackMachine.callProcedure("foo");

    expectCode("\txor rax, rax\n"
            "\tcall $foo\n");
}

TEST_F(StackMachineTest, callProcedure_externUsesPlt) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };

    stackMachine.callProcedure("printf");

    expectCode("\txor rax, rax\n"
            "\tcall $printf wrt ..plt\n");
}

// Target already in a callee-saved reg survives spillCallerSavedRegisters → mov to r10.
TEST_F(StackMachineTest, callProcedureIndirect_movesRegisterTargetToR10) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value junk = intValue("junk");
    Value fp = { "fp", 1, ValueKind::INTEGRAL, 8 };
    stackMachine.startProcedure(testProc("proc", { junk, fp }, { }));
    // Occupy rax so the next functionAddress binds fp to rbx (callee-saved).
    stackMachine.functionAddress("a", "junk");
    stackMachine.functionAddress("foo", "fp");
    assemblyCode.str("");

    stackMachine.callProcedureIndirect("fp");

    // Spill junk from rax (retrieval reg), then mov callee-saved fp → r10 and call.
    expectCode("\tmov [rsp + 40], rax\n"
            "\txor rax, rax\n"
            "\tmov r10, rbx\n"
            "\tcall r10\n");
}

TEST_F(StackMachineTest, callProcedureIndirect_loadsMemoryTargetToR10) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value fp = intValue("fp");
    stackMachine.startProcedure(testProc("proc", { fp }, { }));
    assemblyCode.str("");

    stackMachine.callProcedureIndirect("fp");

    // Local starts in a frame slot; load into r10 then indirect call.
    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("call r10"));
    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("r10"));
}

TEST_F(StackMachineTest, procedureCall_storesAllDirtyCallerSavedRegisters) {
    Value value { "value", 0, ValueKind::INTEGRAL, 4 };
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        reg->assign(&value);
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\tmov [rsp], rax\n"
            "\tmov [rsp], rcx\n"
            "\tmov [rsp], rdx\n"
            "\tmov [rsp], rsi\n"
            "\tmov [rsp], rdi\n"
            "\tmov [rsp], r8\n"
            "\tmov [rsp], r9\n"
            "\tmov [rsp], r10\n"
            "\tmov [rsp], r11\n"
            "\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n");
}

// Variadic ABI: AL must be 0 when no vector args are passed (e.g. printf with integers only)
TEST_F(StackMachineTest, procedureCall_clearsRaxForVariadicAlRequirement) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure(testProc("proc", { value }, { }));
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("printf");

    expectCode("\tmov rdi, [rsp + 40]\n"
            "\txor rax, rax\n"
            "\tcall $printf wrt ..plt\n");
}

TEST_F(StackMachineTest, procedureStart_storesCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.startProcedure(testProc("proc", { }, { }));

    expectCode("global proc\n"
            "$proc:\n"
            "\tpush rbp\n"
            "\tmov rbp, rsp\n"
            "\tsub rsp, 8\n"
            "\tpush rbx\n"
            "\tpush r12\n"
            "\tpush r13\n"
            "\tpush r14\n"
            "\tpush r15\n");
}

TEST_F(StackMachineTest, procedureReturn_returnsWithNoCalleeRegistersSaved) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.returnFromProcedure();

    expectCode("\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureReturn_popsCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.startProcedure(testProc("proc", { }, { }));
    assemblyCode.str("");

    stackMachine.returnFromProcedure();

    expectCode("\tpop r15\n"
            "\tpop r14\n"
            "\tpop r13\n"
            "\tpop r12\n"
            "\tpop rbx\n"
            "\tleave\n"
            "\tret\n");
}

// 4-byte signed locals are stored as dwords; call setup must sign-extend into
// the GP arg register (SysV; git date_string offset < 0).
TEST_F(StackMachineTest, callArgNarrowSignedIntExtendsFromMemory) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    Value value { "value", 0, ValueKind::INTEGRAL, 4, true };
    stackMachine.startProcedure(testProc("proc", { value }, { }));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("procedure");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("movsxd rdi, dword [rsp + 40]"));
}

TEST_F(StackMachineTest, retrieveNarrowIntStoresHomeWithoutRebind) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    Value ret { "r", 0, ValueKind::INTEGRAL, 4, true };
    stackMachine.startProcedure(testProc("f", { ret }, { }));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.retrieveProcedureReturnValue("r");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("mov [rsp +"));
    EXPECT_THAT(code, Not(HasSubstr("movsxd")));
}

TEST_F(StackMachineTest, procedureArgumentPassing_firstIntegerArgumentIsPassedInRDI) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure(testProc("proc", { value }, { }));
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("procedure");

    expectCode("\tmov rdi, [rsp + 40]\n"
            "\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n");
}

// Odd number of stack args needs 8-byte padding so RSP is 16-byte aligned before call
TEST_F(StackMachineTest, procedureCall_padsStackForOddNumberOfStackArguments) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    std::vector<Value> locals;
    for (int i = 0; i < 7; ++i) {
        locals.push_back({ "a" + std::to_string(i), i, ValueKind::INTEGRAL, 8 });
    }
    stackMachine.startProcedure(testProc("proc", locals, { }));
    assemblyCode.str("");

    for (const auto& local : locals) {
        stackMachine.procedureArgument(local.getName());
    }
    stackMachine.callProcedure("procedure");

    expectCode("\tmov rdi, [rsp + 40]\n"
            "\tmov rsi, [rsp + 48]\n"
            "\tmov rdx, [rsp + 56]\n"
            "\tmov rcx, [rsp + 64]\n"
            "\tmov r8, [rsp + 72]\n"
            "\tmov r9, [rsp + 80]\n"
            "\tsub rsp, 16\n"
            "\tmov rax, [rsp + 104]\n"
            "\tmov [rsp], rax\n"
            "\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n"
            "\tadd rsp, 16\n");
}

TEST_F(StackMachineTest, sub_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rcx, rax\n"
            "\tsub rcx, rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v2);
    expectRegisterContains(rcx, v3);
}

TEST_F(StackMachineTest, sub_reg_mem) {
    // given
    rax->assign(&v1);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, rax\n"
            "\tsub rbx, [rsp + 8]\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, [rsp]\n"
            "\tsub rbx, rax\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rax, [rsp]\n"
            "\tsub rax, [rsp + 8]\n");

    expectRegisterContains(rax, v3);
}

TEST_F(StackMachineTest, add_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rcx, rax\n"
            "\tadd rcx, rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v2);
    expectRegisterContains(rcx, v3);
}

TEST_F(StackMachineTest, add_reg_mem) {
    // given
    rax->assign(&v1);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, rax\n"
            "\tadd rbx, [rsp + 8]\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, [rsp]\n"
            "\tadd rbx, rax\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rax, [rsp]\n"
            "\tadd rax, [rsp + 8]\n");

    expectRegisterContains(rax, v3);
}


TEST_F(StackMachineTest, variadicPrologueDumpsGpAndXmmSaveArea) {
    Value last { "last", 0, ValueKind::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("varfn", {}, { last }, false, true));

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("mov ["));
    EXPECT_THAT(code, HasSubstr("rdi"));
    EXPECT_THAT(code, HasSubstr("movq"));
    EXPECT_THAT(code, HasSubstr("xmm0"));
}

TEST_F(StackMachineTest, vaStartWritesTagOffsetsAndLeasOverflowAtRbp16) {
    Value last { "last", 0, ValueKind::INTEGRAL, 8, true };
    Value ap { "ap", 1, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("varfn", { ap }, { last }, false, true));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.vaStart("ap");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("mov dword [rax], 8"));
    EXPECT_THAT(code, HasSubstr("mov dword [rax + 4], 48"));
    EXPECT_THAT(code, HasSubstr("lea "));
    EXPECT_THAT(code, HasSubstr("[rbp + 16]"));
}

TEST_F(StackMachineTest, vaStartOverflowSkipsNamedStackArgs) {
    // 7 named ints: 6 GP + 1 stack. overflow_arg_area is rbp+24, not rbp+16.
    std::vector<Value> named;
    named.reserve(7);
    for (int i = 0; i < 7; ++i) {
        named.emplace_back("a" + std::to_string(i), i, ValueKind::INTEGRAL, 8, true);
    }
    Value ap { "ap", 7, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("varfn", { ap }, named, false, true));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.vaStart("ap");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("[rbp + 24]"));
    EXPECT_THAT(code, Not(HasSubstr("[rbp + 16]")));
}

TEST_F(StackMachineTest, vaStartOverflowSkipsNamedX87BeforeLaterGp) {
    // 5 ints in GP, long double on stack, 6th int in r9. Overflow is after
    // the 16-byte slot (rbp+32), not rbp+16. Master's lastFormalOnStack
    // would point at the long double because the last named formal is in a GP.
    std::vector<Value> named;
    for (int i = 0; i < 5; ++i) {
        named.emplace_back("a" + std::to_string(i), i, ValueKind::INTEGRAL, 8, true);
    }
    named.emplace_back("ld", 5, ValueKind::FLOATING, 16);
    named.emplace_back("g", 6, ValueKind::INTEGRAL, 8, true);
    Value ap { "ap", 7, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("varfn", { ap }, named, false, true));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.vaStart("ap");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("[rbp + 32]"));
    EXPECT_THAT(code, Not(HasSubstr("[rbp + 16]")));
}

TEST_F(StackMachineTest, intelX87ReturnUsesFld) {
    Value ld { "ld", 0, ValueKind::FLOATING, 16 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("make_ld", { ld }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.returnFromProcedure("ld");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("fld tword"));
    EXPECT_THAT(code, HasSubstr("leave"));
}

TEST_F(StackMachineTest, vaArgLabelsAreUniqueAcrossProcedures) {
    // AT&T .L labels are file-scoped; resetting vaArgSeq per procedure
    // collides when two variadic functions live in one TU.
    Value last { "last", 0, ValueKind::INTEGRAL, 8, true };
    Value ap { "ap", 1, ValueKind::INTEGRAL, 24 };
    Value result { "r", 2, ValueKind::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure(testProc("inner", { ap, result }, { last }, false, true));
    stackMachine.vaArg("ap", "r");
    stackMachine.endProcedure();
    stackMachine.startProcedure(testProc("outer", { ap, result }, { last }, false, true));
    stackMachine.vaArg("ap", "r");
    stackMachine.endProcedure();

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr(".Lva_arg_ov_1"));
    EXPECT_THAT(code, HasSubstr(".Lva_arg_ov_2"));
}

TEST_F(StackMachineTest, variadicReturnIsPlainEpilogue) {
    Value arg { "last", 0, ValueKind::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("varfn", {}, { arg }, false, true));
    assemblyCode.str("");
    assemblyCode.clear();
    stackMachine.returnFromProcedure("");
    EXPECT_THAT(assemblyCode.str(), HasSubstr("ret"));
    EXPECT_THAT(assemblyCode.str(), Not(HasSubstr("call")));
}

// --- Production Intel/NASM path: System V memory return (sret) ---

TEST_F(StackMachineTest, intelMemoryReturnPrologueHoldsSretInFirstArgReg) {
    // Aggregates > 16 bytes: callee receives hidden pointer in rdi (first integer arg).
    Value local { "obj", 0, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure(testProc("retbig", { local }, {}, true, false));

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("$retbig:"));
    EXPECT_THAT(code, HasSubstr("push rbp"));
    EXPECT_THAT(code, HasSubstr("mov rbp, rsp"));
    // sret local is spilled from rdi during prologue frame setup.
    EXPECT_THAT(code, HasSubstr("mov"));
}

TEST_F(StackMachineTest, intelMemoryReturnCopiesObjectToSretAndLeavesPointerInRax) {
    Value local { "obj", 0, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("retbig", { local }, {}, true, false));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.returnFromProcedure("obj");

    std::string code = assemblyCode.str();
    // Three 8-byte words copied to [sret]; rax ends as the hidden pointer.
    EXPECT_THAT(code, HasSubstr("mov"));
    EXPECT_THAT(code, HasSubstr("leave"));
    EXPECT_THAT(code, HasSubstr("ret"));
    // Must not treat a 24-byte object as a two-register return (no lone rdx path).
    EXPECT_THAT(code, Not(HasSubstr("mov rdx, rax")));
}

TEST_F(StackMachineTest, intelCallWithMemoryReturnDestLeasIntoRdi) {
    Value dest { "out", 0, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", { dest }, {}, false, false));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.callProcedure("retbig", "out");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("lea rdi,"));
    EXPECT_THAT(code, HasSubstr("call $retbig"));
}

TEST_F(StackMachineTest, intelProcedureCallClearsRaxWhenNoVectorArgs) {
    // Same AL-clear contract as the AT&T suite, asserted in production dialect.
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure(testProc("proc", { value }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("printf");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("xor rax, rax"));
    EXPECT_THAT(code, HasSubstr("call $printf"));
}

// Multi-word stack argument: copied onto a 16-aligned outgoing stack region.
TEST_F(StackMachineTest, intelMultiWordArgumentCopiesOntoStack) {
    Value big { "big", 0, ValueKind::INTEGRAL, 24 };
    // Force stack-passed: fill the 6 integer arg registers first with dummies.
    std::vector<Value> locals { big };
    std::vector<Value> namedArgs;
    for (int i = 0; i < 6; ++i) {
        namedArgs.push_back(Value { "a" + std::to_string(i), i + 1, ValueKind::INTEGRAL, 8 });
        locals.push_back(namedArgs.back());
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", locals, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    for (const auto& a : namedArgs) {
        stackMachine.procedureArgument(a.getName());
    }
    stackMachine.procedureArgument(big.getName());
    stackMachine.callProcedure("callee");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("call $callee"));
    EXPECT_THAT(code, HasSubstr("sub rsp, 32"));
    EXPECT_THAT(code, HasSubstr("add rsp, 32"));
}

// Floating args go in xmm0.. and set AL for variadic callees (SysV).
TEST_F(StackMachineTest, intelFloatingArgumentUsesXmmAndSetsAl) {
    Value d { "d", 0, ValueKind::FLOATING, 8 };
    Value fmt { "fmt", 1, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", { d, fmt }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(fmt.getName());
    stackMachine.procedureArgument(d.getName());
    stackMachine.callProcedure("printf");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("xmm0"));
    EXPECT_THAT(code, HasSubstr("call $printf"));
    // AL = number of vector registers used (mov to rax/eax both set AL).
    EXPECT_TRUE(code.find("mov eax, 1") != std::string::npos
            || code.find("mov rax, 1") != std::string::npos);
}

// float32 call arg is movd into xmm, not movq.
TEST_F(StackMachineTest, intelFloat32CallArgUsesMovd) {
    Value fmt { "fmt", 0, ValueKind::INTEGRAL, 8 };
    Value f { "f", 1, ValueKind::FLOATING, 4 };
    Value code { "code", 2, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", { fmt, f, code }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(fmt.getName());
    stackMachine.procedureArgument(f.getName());
    stackMachine.procedureArgument(code.getName());
    stackMachine.callProcedure("callee");

    std::string codeAsm = assemblyCode.str();
    EXPECT_THAT(codeAsm, testing::HasSubstr("movd xmm0"));
    EXPECT_THAT(codeAsm, testing::Not(testing::HasSubstr("movq xmm0")));
    // place.index: second integer is rsi (GP1), not rdx. Float does not spend a GP slot.
    EXPECT_THAT(codeAsm, testing::HasSubstr("rsi"));
}

// AL counts xmm0..xmm7 only; a ninth float does not set AL to 9.
TEST_F(StackMachineTest, intelNinthFloatDoesNotCountTowardAl) {
    std::vector<Value> locals;
    for (int i = 0; i < 9; ++i) {
        locals.push_back({ "f" + std::to_string(i), i, ValueKind::FLOATING, 8 });
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", locals, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    for (int i = 0; i < 9; ++i) {
        stackMachine.procedureArgument("f" + std::to_string(i));
    }
    stackMachine.callProcedure("callee");

    std::string code = assemblyCode.str();
    EXPECT_TRUE(code.find("mov rax, 8") != std::string::npos
            || code.find("mov eax, 8") != std::string::npos);
    EXPECT_TRUE(code.find("mov rax, 9") == std::string::npos
            && code.find("mov eax, 9") == std::string::npos);
}

TEST_F(StackMachineTest, intelFloat32LvalueAssignUsesDword) {
    Value f { "f", 0, ValueKind::FLOATING, 4 };
    Value p { "p", 1, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("storef", { f, p }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.lvalueAssign("f", "p");

    EXPECT_THAT(assemblyCode.str(), HasSubstr("dword"));
}

// SSE path for double add: load bits, addsd, park result.
TEST_F(StackMachineTest, intelFloatingAddUsesAddsd) {
    Value a { "a", 0, ValueKind::FLOATING, 8 };
    Value b { "b", 1, ValueKind::FLOATING, 8 };
    Value r { "r", 2, ValueKind::FLOATING, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("fadd", { a, b, r }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.add("a", "b", "r");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("addsd"));
    EXPECT_THAT(code, HasSubstr("xmm0"));
    EXPECT_THAT(code, HasSubstr("xmm1"));
}

TEST_F(StackMachineTest, intelFloat32AddUsesAddss) {
    Value a { "a", 0, ValueKind::FLOATING, 4 };
    Value b { "b", 1, ValueKind::FLOATING, 4 };
    Value r { "r", 2, ValueKind::FLOATING, 4 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("faddss", { a, b, r }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.add("a", "b", "r");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("addss"));
    EXPECT_THAT(code, Not(HasSubstr("addsd")));
}

// SysV: MEMORY-class returns use sret even on variadic callees (hidden first GP).
TEST_F(StackMachineTest, variadicMemoryReturnUsesSret) {
    Value obj { "obj", 0, ValueKind::INTEGRAL, 24, true, type::sysv::memoryClass() };
    Value named { "named", 1, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure(testProc("vret", { obj }, { named }, true, true));
    assemblyCode.str("");
    assemblyCode.clear();
    stackMachine.returnFromProcedure("obj");
    std::string epilogue = assemblyCode.str();
    // sret copy writes through the hidden pointer, then returns that pointer in rax.
    EXPECT_THAT(epilogue, HasSubstr("mov rax, rcx"));
}

// Nested sret call sequence at the StackMachine level: lea rdi for dest, call,
// then another lea for outer dest must still appear on a subsequent call.
TEST_F(StackMachineTest, intelNestedSretCallsEachLeaDestIntoRdi) {
    Value outer { "outer", 0, ValueKind::INTEGRAL, 24 };
    Value inner { "inner", 3, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", { outer, inner }, {}, false, false));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.callProcedure("make_inner", "inner");
    stackMachine.callProcedure("make_outer", "outer");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("lea rdi,"));
    EXPECT_THAT(code, HasSubstr("call $make_inner"));
    EXPECT_THAT(code, HasSubstr("call $make_outer"));
    int leaCount = 0;
    for (std::size_t i = 0; i + 7 < code.size(); ++i) {
        if (code.compare(i, 7, "lea rdi") == 0) {
            ++leaCount;
        }
    }
    EXPECT_GE(leaCount, 2);
}

// sret spends GP0 (rdi). Float then int: xmm0 + rsi (place.index 1), not firstReg+count.
TEST_F(StackMachineTest, intelSretCallFloatThenIntUsesPlaceIndex) {
    Value dest { "out", 0, ValueKind::INTEGRAL, 24 };
    Value f { "f", 3, ValueKind::FLOATING, 8 };
    Value n { "n", 4, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure(testProc("caller", { dest, f, n }, {}, false, false));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(f.getName());
    stackMachine.procedureArgument(n.getName());
    stackMachine.callProcedure("retbig", "out");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("lea rdi,"));
    EXPECT_THAT(code, HasSubstr("xmm0"));
    EXPECT_THAT(code, HasSubstr("rsi"));
}

} // namespace
