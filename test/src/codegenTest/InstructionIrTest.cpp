#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <sstream>
#include <string>
#include <vector>

#include "codegen/Amd64Registers.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/ATandTInstructionSet.h"
#include "codegen/Instruction.h"
#include "codegen/StackMachine.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"

namespace {

using namespace testing;
using namespace codegen;

Procedure makeProc(std::string name, std::vector<Instruction> body,
        ProcedureFrame frame = {}) {
    Procedure p;
    p.name = std::move(name);
    p.frame = std::move(frame);
    p.body = std::move(body);
    return p;
}

IntermediateRepresentation arithmeticSequence() {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("main", {
            ir::assignConstant("1", "t0"),
            ir::assignConstant("2", "t1"),
            ir::add("t0", "t1", "t2"),
            ir::sub("t2", "t0", "t3"),
            ir::mul("t3", "t1", "t4"),
            ir::div("t4", "t1", "t5"),
            ir::mod("t5", "t0", "t6"),
            ir::shl("t6", "t0", "t7"),
            ir::shr("t7", "t0", "t8"),
            ir::andOp("t8", "t1", "t9"),
            ir::orOp("t9", "t0", "t10"),
            ir::xorOp("t10", "t1", "t11"),
            ir::unaryMinus("t11", "t12"),
            ir::unaryNot("t12", "t13"),
    }));
    return ir;
}

const char* kArithmeticDump =
        "PROC main\n"
        "\tt0 := 1\n"
        "\tt1 := 2\n"
        "\tt2 := t0 + t1\n"
        "\tt3 := t2 - t0\n"
        "\tt4 := t3 * t1\n"
        "\tt5 := t4 / t1\n"
        "\tt6 := t5 % t0\n"
        "\tt7 := t6 << t0\n"
        "\tt8 := t7 >> t0\n"
        "\tt9 := t8 AND t1\n"
        "\tt10 := t9 OR t0\n"
        "\tt11 := t10 XOR t1\n"
        "\tt12 := -t11\n"
        "\tt13 := ~t12\n"
        "ENDPROC main\n";

IntermediateRepresentation controlFlowSequence() {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("main", {
            ir::zeroCompare("x"),
            ir::jump("else", JumpCondition::IF_EQUAL),
            ir::assignConstant("1", "r"),
            ir::jump("end"),
            ir::label("else"),
            ir::assignConstant("0", "r"),
            ir::label("end"),
            ir::valueCompare("a", "b"),
            ir::jump("loop", JumpCondition::IF_BELOW),
            ir::jump("loop", JumpCondition::IF_ABOVE),
            ir::jump("loop", JumpCondition::IF_BELOW_OR_EQUAL),
            ir::jump("loop", JumpCondition::IF_ABOVE_OR_EQUAL),
            ir::jump("loop", JumpCondition::IF_NOT_EQUAL),
            ir::label("loop"),
            ir::inc("i"),
            ir::dec("i"),
            ir::inc("p", 4),
            ir::dec("p", 8),
    }));
    return ir;
}

const char* kControlFlowDump =
        "PROC main\n"
        "\tCMP x, 0\n"
        "\tJE else\n"
        "\tr := 1\n"
        "\tGOTO end\n"
        "else:\n"
        "\tr := 0\n"
        "end:\n"
        "\tCMP a, b\n"
        "\tJB loop\n"
        "\tJA loop\n"
        "\tJBE loop\n"
        "\tJAE loop\n"
        "\tJNE loop\n"
        "loop:\n"
        "\tINC i\n"
        "\tDEC i\n"
        "\tp := p + 4\n"
        "\tp := p - 8\n"
        "ENDPROC main\n";

IntermediateRepresentation callReturnSequence() {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("foo",
            {
                    ir::argument("n"),
                    ir::argument("t0"),
                    ir::call("printf"),
                    ir::retrieve("ret"),
                    ir::call("fp", true),
                    ir::ret("ret"),
                    ir::voidReturn(),
            },
            ProcedureFrame {
                    { codegen::Value { "t0", 0, codegen::Type::INTEGRAL, 8 } },
                    { codegen::Value { "n", 0, codegen::Type::INTEGRAL, 4 } },
            }));
    return ir;
}

const char* kCallReturnDump =
        "PROC foo\n"
        "\tPARAM n\n"
        "\tPARAM t0\n"
        "\tCALL printf\n"
        "\tRETRIEVE ret\n"
        "\tCALL *fp\n"
        "\tRETURN ret\n"
        "\tRETURN\n"
        "ENDPROC foo\n";

IntermediateRepresentation vaSequence() {
    IntermediateRepresentation ir;
    Procedure p = makeProc("sum",
            {
                    ir::vaStart("ap", "n"),
                    ir::vaArg("ap", "t0"),
                    ir::vaCopy("cp", "ap"),
                    ir::vaEnd(),
                    ir::ret("t0"),
            },
            ProcedureFrame {
                    { codegen::Value { "ap", 0, codegen::Type::INTEGRAL, 24 },
                            codegen::Value { "cp", 3, codegen::Type::INTEGRAL, 24 },
                            codegen::Value { "t0", 6, codegen::Type::INTEGRAL, 4 } },
                    { codegen::Value { "n", 0, codegen::Type::INTEGRAL, 4 } },
            });
    p.variadic = true;
    ir.procedures.push_back(std::move(p));
    return ir;
}

const char* kVaDump =
        "PROC sum variadic\n"
        "\tVA_START ap, n\n"
        "\tVA_ARG ap -> t0\n"
        "\tVA_COPY cp, ap\n"
        "\tVA_END\n"
        "\tRETURN t0\n"
        "ENDPROC sum\n";

IntermediateRepresentation memorySequence() {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("main", {
            ir::addressOf("x", "p"),
            ir::dereference("p", "lv", "v"),
            ir::lvalueAssign("v", "p"),
            ir::assign("v", "w"),
            ir::assignLabelAddress("str0", "s"),
            ir::functionAddress("foo", "fp"),
            ir::indexAddress("arr", "i", 4, "a1", symbols::AddressBaseMode::LeaObject),
            ir::indexAddress("ptr", "j", 8, "a2", symbols::AddressBaseMode::PointerValue),
            ir::fieldAddress("obj", 16, "f1", symbols::AddressBaseMode::LeaObject),
            ir::fieldAddress("po", 8, "f2", symbols::AddressBaseMode::PointerValue),
            ir::pointerOffset("p", "k", 4, "p2", false),
            ir::pointerOffset("p", "k", 1, "p3", true),
            ir::pointerDiff("p2", "p", 4, "d"),
            ir::pointerDiff("p2", "p", 1, "d2"),
    }));
    return ir;
}

const char* kMemoryDump =
        "PROC main\n"
        "\tp := &x\n"
        "\tv := *p\n"
        "\t*p := v\n"
        "\tw := v\n"
        "\ts := &str0\n"
        "\tfp := &foo (function)\n"
        "\ta1 := &arr[i] stride=4 (array)\n"
        "\ta2 := &ptr[j] stride=8 (ptr)\n"
        "\tf1 := &(obj.16)\n"
        "\tf2 := &(po->8)\n"
        "\tp2 := p + k*4 (ptr)\n"
        "\tp3 := p - k (ptr)\n"
        "\td := (p2 - p) /4 (ptrdiff)\n"
        "\td2 := (p2 - p) (ptrdiff)\n"
        "ENDPROC main\n";

TEST(InstructionIr, freezesArithmeticDump) {
    EXPECT_THAT(toString(arithmeticSequence()), StrEq(kArithmeticDump));
}

TEST(InstructionIr, freezesControlFlowDump) {
    EXPECT_THAT(toString(controlFlowSequence()), StrEq(kControlFlowDump));
}

TEST(InstructionIr, freezesCallReturnDump) {
    EXPECT_THAT(toString(callReturnSequence()), StrEq(kCallReturnDump));
}

TEST(InstructionIr, freezesVaDump) {
    EXPECT_THAT(toString(vaSequence()), StrEq(kVaDump));
}

TEST(InstructionIr, freezesMemoryDump) {
    EXPECT_THAT(toString(memorySequence()), StrEq(kMemoryDump));
}

TEST(InstructionIr, procedurePreservesFrame) {
    auto ir = callReturnSequence();
    ASSERT_THAT(ir.procedures, SizeIs(1));
    EXPECT_THAT(ir.procedures[0].name, Eq("foo"));
    ASSERT_THAT(ir.procedures[0].frame.locals, SizeIs(1));
    EXPECT_THAT(ir.procedures[0].frame.locals[0].getName(), Eq("t0"));
    ASSERT_THAT(ir.procedures[0].frame.arguments, SizeIs(1));
    EXPECT_THAT(ir.procedures[0].frame.arguments[0].getName(), Eq("n"));
}

TEST(InstructionIr, dereferencePreservesLvalue) {
    Instruction i = ir::dereference("p", "lvalue_tmp", "v");
    EXPECT_THAT(i.op, Eq(Op::Dereference));
    EXPECT_THAT(i.arg0, Eq("p"));
    EXPECT_THAT(i.arg1, Eq("lvalue_tmp"));
    EXPECT_THAT(i.result, Eq("v"));
}

TEST(InstructionIr, callIndirectAndPointerSubtractAreSeparate) {
    EXPECT_FALSE(ir::call("direct").callIndirect);
    EXPECT_TRUE(ir::call("indirect", true).callIndirect);
    EXPECT_FALSE(ir::call("direct").pointerSubtract);

    Instruction sub = ir::pointerOffset("p", "k", 4, "r", true);
    EXPECT_TRUE(sub.pointerSubtract);
    EXPECT_FALSE(sub.callIndirect);
}

TEST(InstructionIr, preambleDeclaresReferencedExternsOnly) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("main",
            { ir::argument("fmt"), ir::call("printf"), ir::ret("t0") },
            ProcedureFrame { { codegen::Value { "fmt", 0, codegen::Type::INTEGRAL, 8 },
                    codegen::Value { "t0", 1, codegen::Type::INTEGRAL, 8 } }, {} }));

    std::ostringstream assembly;
    AssemblyGenerator generator { std::make_unique<StackMachine>(
            &assembly, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>()) };
    generator.generateAssemblyCode(ir, {}, {});

    const std::string code = assembly.str();
    EXPECT_THAT(code, HasSubstr(".extern printf"));
    EXPECT_THAT(code, Not(HasSubstr(".extern strtod")));
    EXPECT_THAT(code, Not(HasSubstr(".extern main")));
}

TEST(InstructionIr, assemblyGeneratorEmitsFromDataIr) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("main",
            { ir::assignConstant("0", "t0"), ir::ret("t0") },
            ProcedureFrame { { codegen::Value { "t0", 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    std::ostringstream assembly;
    AssemblyGenerator generator { std::make_unique<StackMachine>(
            &assembly, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>()) };
    generator.generateAssemblyCode(ir, {}, {});

    const std::string code = assembly.str();
    EXPECT_THAT(code, HasSubstr("main:"));
    EXPECT_THAT(code, HasSubstr("ret"));
}

TEST(InstructionIr, emitIsDeterministic) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("foo",
            { ir::assignConstant("42", "t0"), ir::ret("t0") },
            ProcedureFrame { { codegen::Value { "t0", 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    auto emitAsm = [](const IntermediateRepresentation& program) {
        std::ostringstream assembly;
        AssemblyGenerator generator { std::make_unique<StackMachine>(
                &assembly, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>()) };
        generator.generateAssemblyCode(program, {}, {});
        return assembly.str();
    };

    const std::string once = emitAsm(ir);
    EXPECT_THAT(once, StrEq(emitAsm(ir)));
    EXPECT_THAT(once, HasSubstr("foo"));
    EXPECT_THAT(once, HasSubstr("ret"));
}

} // namespace
