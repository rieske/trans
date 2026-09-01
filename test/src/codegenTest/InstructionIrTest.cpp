#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "codegen/Amd64Registers.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/ATandTInstructionSet.h"
#include "codegen/Cfg.h"
#include "codegen/Instruction.h"
#include "codegen/IrPasses.h"
#include "codegen/StackMachine.h"
#include "codegen/SysVCallConv.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"
#include "types/ObjectAbi.h"

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

Procedure makeProc(IrStringTable& strings, std::string_view name, std::vector<Instruction> body,
        ProcedureFrame frame = {}, bool memoryReturn = false, bool variadic = false) {
    Procedure p;
    p.name = strings.intern(name);
    p.frame = std::move(frame);
    p.body = std::move(body);
    p.memoryReturn = memoryReturn;
    p.variadic = variadic;
    internProcedureTemps(strings, p);
    return p;
}

std::size_t countSubstr(const std::string& haystack, const std::string& needle) {
    std::size_t n = 0;
    for (std::size_t pos = 0; (pos = haystack.find(needle, pos)) != std::string::npos;
            pos += needle.size()) {
        ++n;
    }
    return n;
}

IntermediateRepresentation arithmeticSequence() {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main", {
            ir::assignConstant(n("1"), n("t0")),
            ir::assignConstant(n("2"), n("t1")),
            ir::add(n("t0"), n("t1"), n("t2")),
            ir::sub(n("t2"), n("t0"), n("t3")),
            ir::mul(n("t3"), n("t1"), n("t4")),
            ir::div(n("t4"), n("t1"), n("t5")),
            ir::mod(n("t5"), n("t0"), n("t6")),
            ir::shl(n("t6"), n("t0"), n("t7")),
            ir::shr(n("t7"), n("t0"), n("t8")),
            ir::andOp(n("t8"), n("t1"), n("t9")),
            ir::orOp(n("t9"), n("t0"), n("t10")),
            ir::xorOp(n("t10"), n("t1"), n("t11")),
            ir::unaryMinus(n("t11"), n("t12")),
            ir::unaryNot(n("t12"), n("t13")),
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
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main", {
            ir::zeroCompare(n("x")),
            ir::jump(n("else"), JumpCondition::IF_EQUAL),
            ir::assignConstant(n("1"), n("r")),
            ir::jump(n("end")),
            ir::label(n("else")),
            ir::assignConstant(n("0"), n("r")),
            ir::label(n("end")),
            ir::valueCompare(n("a"), n("b")),
            ir::jump(n("loop"), JumpCondition::IF_BELOW),
            ir::jump(n("loop"), JumpCondition::IF_ABOVE),
            ir::jump(n("loop"), JumpCondition::IF_BELOW_OR_EQUAL),
            ir::jump(n("loop"), JumpCondition::IF_ABOVE_OR_EQUAL),
            ir::jump(n("loop"), JumpCondition::IF_NOT_EQUAL),
            ir::label(n("loop")),
            ir::inc(n("i")),
            ir::dec(n("i")),
            ir::inc(n("p"), 4),
            ir::dec(n("p"), 8),
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
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "foo",
            {
                    ir::argument(n("n")),
                    ir::argument(n("t0")),
                    ir::call(n("printf")),
                    ir::retrieve(n("ret")),
                    ir::call(n("fp"), true),
                    ir::ret(n("ret")),
                    ir::voidReturn(),
            },
            ProcedureFrame {
                    { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } },
                    { codegen::Value { n("n"), 0, codegen::Type::INTEGRAL, 4 } },
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
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "sum",
            {
                    ir::vaStart(n("ap"), n("n")),
                    ir::vaArg(n("ap"), n("t0")),
                    ir::vaCopy(n("cp"), n("ap")),
                    ir::vaEnd(),
                    ir::ret(n("t0")),
            },
            ProcedureFrame {
                    { codegen::Value { n("ap"), 0, codegen::Type::INTEGRAL, 24 },
                            codegen::Value { n("cp"), 3, codegen::Type::INTEGRAL, 24 },
                            codegen::Value { n("t0"), 6, codegen::Type::INTEGRAL, 4 } },
                    { codegen::Value { n("n"), 0, codegen::Type::INTEGRAL, 4 } },
            },
            false, true));
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
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main", {
            ir::addressOf(n("x"), n("p")),
            ir::dereference(n("p"), n("lv"), n("v")),
            ir::lvalueAssign(n("v"), n("p")),
            ir::assign(n("v"), n("w")),
            ir::assignLabelAddress(n("str0"), n("s")),
            ir::functionAddress(n("foo"), n("fp")),
            ir::indexAddress(n("arr"), n("i"), 4, n("a1"), symbols::AddressBaseMode::LeaObject),
            ir::indexAddress(n("ptr"), n("j"), 8, n("a2"), symbols::AddressBaseMode::PointerValue),
            ir::fieldAddress(n("obj"), 16, n("f1"), symbols::AddressBaseMode::LeaObject),
            ir::fieldAddress(n("po"), 8, n("f2"), symbols::AddressBaseMode::PointerValue),
            ir::pointerOffset(n("p"), n("k"), 4, n("p2"), false),
            ir::pointerOffset(n("p"), n("k"), 1, n("p3"), true),
            ir::pointerDiff(n("p2"), n("p"), 4, n("d")),
            ir::pointerDiff(n("p2"), n("p"), 1, n("d2")),
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

IntermediateRepresentation widenCopyBuiltinSequence() {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main", {
            ir::widen(n("x"), n("w"), true),
            ir::copyPart(n("src"), n("dst"), 8),
            ir::bswap(n("a"), n("b"), 4),
            ir::ctz(n("c"), n("d"), 8),
            ir::allocaBytes(n("n"), n("p")),
    }));
    return ir;
}

const char* kWidenCopyBuiltinDump =
        "PROC main\n"
        "\tw := widen x\n"
        "\tdst := src[+8]\n"
        "\tBSWAP4 a -> b\n"
        "\tCTZ8 c -> d\n"
        "\tALLOCA n -> p\n"
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

TEST(InstructionIr, freezesWidenCopyBuiltinDump) {
    EXPECT_THAT(toString(widenCopyBuiltinSequence()), StrEq(kWidenCopyBuiltinDump));
}

TEST(Instruction, classifiesLabelTerminatorOrdinary) {
    EXPECT_EQ(instructionClass(Op::Label), InstructionClass::Label);
    EXPECT_EQ(instructionClass(Op::Jump), InstructionClass::Terminator);
    EXPECT_EQ(instructionClass(Op::Return), InstructionClass::Terminator);
    EXPECT_EQ(instructionClass(Op::VoidReturn), InstructionClass::Terminator);
    EXPECT_EQ(instructionClass(Op::Call), InstructionClass::Ordinary);
    EXPECT_EQ(instructionClass(Op::Add), InstructionClass::Ordinary);
    EXPECT_TRUE(instructionTransfersControl(ir::voidReturn()));
    EXPECT_TRUE(instructionTransfersControl(ir::jump(0)));
    EXPECT_FALSE(instructionTransfersControl(ir::call(0)));
    EXPECT_FALSE(instructionTransfersControl(ir::label(0)));
}

TEST(Instruction, buildersHaveNoUnusedFields) {
    validateInstruction(ir::add(0, 1, 2));
    validateInstruction(ir::div(0, 1, 2, false));
    validateInstruction(ir::shr(0, 1, 2, false));
    validateInstruction(ir::inc(0, 4));
    validateInstruction(ir::widen(0, 1, true));
    validateInstruction(ir::assignConstant(0, 1));
    validateInstruction(ir::assignConstant(0, 1, 2));
    validateInstruction(ir::dereference(0, 1, 2));
    validateInstruction(ir::indexAddress(0, 1, 4, 2, symbols::AddressBaseMode::PointerValue));
    validateInstruction(ir::fieldAddress(0, 8, 1));
    validateInstruction(ir::pointerOffset(0, 1, 4, 2, true));
    validateInstruction(ir::valueCompare(0, 1, false));
    validateInstruction(ir::zeroCompare(0));
    validateInstruction(ir::jump(0, JumpCondition::IF_EQUAL, false));
    validateInstruction(ir::label(0));
    validateInstruction(ir::call(0, true, 1));
    validateInstruction(ir::retrieve(0, true));
    validateInstruction(ir::ret(0));
    validateInstruction(ir::voidReturn());
    validateInstruction(ir::vaStart(0, 1));
    validateInstruction(ir::vaEnd());
    validateInstruction(ir::bswap(0, 1, 4));
    validateInstruction(ir::allocaBytes(0, 1));
}

TEST(Instruction, unusedFieldOnVoidReturnIsInvalid) {
    Instruction i = ir::voidReturn();
    i.arg0 = 0;
    EXPECT_THROW(validateInstruction(i), std::logic_error);
    i = ir::voidReturn();
    i.cond = JumpCondition::IF_EQUAL;
    EXPECT_THROW(validateInstruction(i), std::logic_error);
}

TEST(Instruction, implicitBlockShapeAcceptsFallthroughToLabel) {
    validateProcedureBody({
            ir::zeroCompare(0),
            ir::jump(1, JumpCondition::IF_EQUAL),
            ir::inc(2),
            ir::label(1),
            ir::voidReturn(),
    });
}

TEST(Instruction, implicitBlockShapeRejectsCodeAfterTerminator) {
    EXPECT_THROW(validateProcedureBody({ ir::voidReturn(), ir::inc(0) }), std::logic_error);
}

TEST(Instruction, catalogBodiesHaveValidFields) {
    for (auto ir : { arithmeticSequence(), controlFlowSequence(), callReturnSequence(),
                 vaSequence(), memorySequence(), widenCopyBuiltinSequence() }) {
        for (const auto& instruction : ir.procedures[0].body) {
            validateInstruction(instruction);
        }
    }
}

TEST(Instruction, callReturnDumpPinsTerminatorThenTerminator) {
    EXPECT_THROW(validateProcedureBody(callReturnSequence().procedures[0].body), std::logic_error);
}

TEST(InstructionIr, procedureInternsNameAndTemps) {
    IntermediateRepresentation ir;
    Procedure p;
    p.name = ir.strings.intern("f");
    p.memoryReturn = true;
    p.variadic = true;
    internProcedureTemps(ir.strings, p);

    EXPECT_THAT(ir.strings.get(p.name), Eq("f"));
    EXPECT_THAT(ir.strings.get(p.sretId), Eq(type::object_abi::SRET_SYMBOL_NAME));
    ASSERT_THAT(p.vaGpHomes, SizeIs(SYSV_INTEGER_ARG_REGS));
    ASSERT_THAT(p.vaXmmHomes, SizeIs(SYSV_SSE_ARG_REGS));
    EXPECT_THAT(ir.strings.get(p.vaGpHomes[0]), Eq(vaGpHomeName(0)));
    EXPECT_THAT(ir.strings.get(p.vaXmmHomes[0]), Eq(vaXmmHomeName(0)));
}

TEST(InstructionIr, procedurePreservesFrame) {
    auto ir = callReturnSequence();
    ASSERT_THAT(ir.procedures, SizeIs(1));
    EXPECT_THAT(ir.strings.get(ir.procedures[0].name), Eq("foo"));
    ASSERT_THAT(ir.procedures[0].frame.locals, SizeIs(1));
    EXPECT_THAT(ir.strings.get(ir.procedures[0].frame.locals[0].id()), Eq("t0"));
    ASSERT_THAT(ir.procedures[0].frame.arguments, SizeIs(1));
    EXPECT_THAT(ir.strings.get(ir.procedures[0].frame.arguments[0].id()), Eq("n"));
}

TEST(InstructionIr, frameNamesResolveAfterGenerateIrMove) {
    auto ir = runIrPasses(callReturnSequence());
    ASSERT_THAT(ir.procedures, SizeIs(1));
    EXPECT_THAT(ir.strings.get(ir.procedures[0].frame.locals[0].id()), Eq("t0"));
    EXPECT_THAT(ir.strings.get(ir.procedures[0].frame.arguments[0].id()), Eq("n"));
}

TEST(InstructionIr, dereferencePreservesLvalue) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Instruction i = ir::dereference(n("p"), n("lvalue_tmp"), n("v"));
    EXPECT_THAT(i.op, Eq(Op::Dereference));
    EXPECT_THAT(ir.strings.get(i.arg0), Eq("p"));
    EXPECT_THAT(ir.strings.get(i.arg1), Eq("lvalue_tmp"));
    EXPECT_THAT(ir.strings.get(i.result), Eq("v"));
}

TEST(InstructionIr, callIndirectAndPointerSubtractAreSeparate) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    EXPECT_FALSE(ir::call(n("direct")).callIndirect);
    EXPECT_TRUE(ir::call(n("indirect"), true).callIndirect);
    EXPECT_FALSE(ir::call(n("direct")).pointerSubtract);

    Instruction sub = ir::pointerOffset(n("p"), n("k"), 4, n("r"), true);
    EXPECT_TRUE(sub.pointerSubtract);
    EXPECT_FALSE(sub.callIndirect);
}

TEST(InstructionIr, preambleDeclaresReferencedExternsOnly) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main",
            { ir::argument(n("fmt")), ir::call(n("printf")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("fmt"), 0, codegen::Type::INTEGRAL, 8 },
                    codegen::Value { n("t0"), 1, codegen::Type::INTEGRAL, 8 } }, {} }));

    std::ostringstream assembly;
    AssemblyGenerator generator { &assembly, std::make_unique<ATandTInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    generator.generateAssemblyCode(ir, {}, {});

    const std::string code = assembly.str();
    EXPECT_THAT(code, HasSubstr(".extern printf"));
    EXPECT_THAT(code, Not(HasSubstr(".extern strtod")));
    EXPECT_THAT(code, Not(HasSubstr(".extern main")));
}

TEST(InstructionIr, referenceObjectEmitsOneExtern) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main",
            { ir::assignConstant(n("0"), n("t0")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    GlobalVariable x;
    x.name = "x";
    x.sizeInBytes = 4;
    x.emission = ObjectEmission::Reference;

    std::ostringstream assembly;
    AssemblyGenerator generator { &assembly, std::make_unique<ATandTInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    generator.generateAssemblyCode(ir, {}, { x });

    EXPECT_THAT(ir.strings.find("x"), Ne(kNoSymbol));
    EXPECT_THAT(countSubstr(assembly.str(), ".extern x\n"), Eq(1u));
}

TEST(InstructionIr, referenceObjectAddressEmitsOneExtern) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main",
            { ir::assignConstant(n("0"), n("t0")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    GlobalVariable x;
    x.name = "x";
    x.sizeInBytes = 4;
    x.emission = ObjectEmission::Reference;

    GlobalVariable pointer;
    pointer.name = "p";
    pointer.sizeInBytes = 8;
    pointer.initValues = { symbols::StaticAddress { "x" } };

    std::ostringstream assembly;
    AssemblyGenerator generator { &assembly, std::make_unique<ATandTInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    generator.generateAssemblyCode(ir, {}, { x, pointer });

    EXPECT_THAT(countSubstr(assembly.str(), ".extern x\n"), Eq(1u));
}

TEST(InstructionIr, stringPoolAddressIsNotExtern) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main",
            { ir::assignConstant(n("0"), n("t0")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    GlobalVariable pointer;
    pointer.name = "p";
    pointer.sizeInBytes = 8;
    pointer.initValues = { symbols::StaticAddress { "L$str1" } };

    std::ostringstream assembly;
    AssemblyGenerator generator { &assembly, std::make_unique<ATandTInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    generator.generateAssemblyCode(ir, { { "L$str1", "hi" } }, { pointer });

    const std::string code = assembly.str();
    EXPECT_THAT(code, Not(HasSubstr(".extern L$str1")));
    EXPECT_THAT(code, HasSubstr("L$str1"));
}

TEST(InstructionIr, assemblyGeneratorEmitsFromDataIr) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "main",
            { ir::assignConstant(n("0"), n("t0")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    std::ostringstream assembly;
    AssemblyGenerator generator { &assembly, std::make_unique<ATandTInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    generator.generateAssemblyCode(ir, {}, {});

    const std::string code = assembly.str();
    EXPECT_THAT(code, HasSubstr("main:"));
    EXPECT_THAT(code, HasSubstr("ret"));
}

TEST(InstructionIr, emitIsDeterministic) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "foo",
            { ir::assignConstant(n("42"), n("t0")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    auto emitAsm = [](IntermediateRepresentation program) {
        std::ostringstream assembly;
        AssemblyGenerator generator { &assembly, std::make_unique<ATandTInstructionSet>(),
                std::make_unique<Amd64Registers>() };
        generator.generateAssemblyCode(program, {}, {});
        return assembly.str();
    };

    const std::string once = emitAsm(ir);
    EXPECT_THAT(once, StrEq(emitAsm(ir)));
    EXPECT_THAT(once, HasSubstr("foo"));
    EXPECT_THAT(once, HasSubstr("ret"));
}

} // namespace
