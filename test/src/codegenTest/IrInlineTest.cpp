#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "codegen/IrBuilders.h"
#include "codegen/IrInline.h"
#include "codegen/IrStringTable.h"
#include "codegen/Value.h"

#include <string_view>
#include <vector>

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

Procedure makeProc(IrStringTable& strings, std::string_view name, std::vector<Instruction> body,
        ProcedureFrame frame = {}) {
    Procedure p;
    p.name = strings.intern(name);
    p.frame = std::move(frame);
    p.body = std::move(body);
    internProcedureTemps(strings, p);
    return p;
}

codegen::Value integral(IrStringTable& strings, std::string_view name, int size = 4) {
    return codegen::Value { strings.intern(name), 0, Type::INTEGRAL, size };
}

ProcedureFrame oneFormal(IrStringTable& strings, std::string_view name) {
    ProcedureFrame frame;
    frame.arguments.push_back(integral(strings, name));
    return frame;
}

Procedure smallCallee(IrStringTable& strings, IrN n) {
    return makeProc(strings, "add1", {
            ir::assignConstant(n("1"), n("c")),
            ir::add(n("x"), n("c"), n("t")),
            ir::ret(n("t")),
    }, oneFormal(strings, "x"));
}

Procedure smallCaller(IrStringTable& strings, IrN n) {
    return makeProc(strings, "f", {
            ir::argument(n("y")),
            ir::call(n("add1")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    }, oneFormal(strings, "y"));
}

TEST(IrInline, setjmpFamilyIncludesGlibcSpellings) {
    EXPECT_TRUE(isSetjmpFamily("setjmp"));
    EXPECT_TRUE(isSetjmpFamily("__sigsetjmp"));
    EXPECT_TRUE(isSetjmpFamily("__longjmp_chk"));
    EXPECT_FALSE(isSetjmpFamily("add1"));
}

TEST(IrInline, nonLabelCountSkipsLabels) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    EXPECT_THAT(nonLabelCount({
            ir::label(n("L")),
            ir::inc(n("x")),
            ir::inc(n("x")),
            ir::voidReturn(),
    }), Eq(3));
}

TEST(IrInline, callIsEligibleAcceptsSmallFinishedCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    EXPECT_TRUE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesIndirect) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    EXPECT_FALSE(callIsEligible(ir::call(n("add1"), true), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesMissingProcedureName) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    EXPECT_FALSE(callIsEligible(ir::call(n("g")), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesVariadic) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = smallCallee(ir.strings, n);
    callee.variadic = true;
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesSetjmpFamilyCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "wrap", {
            ir::call(n("__sigsetjmp")),
            ir::voidReturn(),
    });
    EXPECT_TRUE(calleeLooksUnsafe(callee, ir.strings));
    EXPECT_FALSE(callIsEligible(ir::call(n("wrap")), caller, callee, ir.strings, {}, 0, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesVaOpInCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "wrap", {
            ir::vaStart(n("ap")),
            ir::voidReturn(),
    });
    EXPECT_TRUE(calleeLooksUnsafe(callee, ir.strings));
    EXPECT_FALSE(callIsEligible(ir::call(n("wrap")), caller, callee, ir.strings, {}, 0, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesUnfinishedCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, {}, 1, 0, 0, false));
}

TEST(IrInline, callIsEligibleRefusesSelfCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure self = smallCallee(ir.strings, n);
    self.body = {
            ir::argument(n("x")),
            ir::call(n("add1")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    };
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), self, self, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesOversizeCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    std::vector<Instruction> body;
    body.reserve(33);
    for (int i = 0; i < 33; ++i) {
        body.push_back(ir::inc(n("x")));
    }
    Procedure callee = makeProc(ir.strings, "add1", std::move(body), oneFormal(ir.strings, "x"));
    EXPECT_THAT(nonLabelCount(callee.body), Eq(33));
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesParamCountMismatch) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, {}, 0, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesSretMismatch) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = smallCallee(ir.strings, n);
    callee.memoryReturn = true;
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, callIsEligibleRefusesCallerCap) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    InlineCaps caps;
    caps.maxInlinesPerCaller = 0;
    EXPECT_FALSE(callIsEligible(ir::call(n("add1")), caller, callee, ir.strings, caps, 1, 0, 0, true));
}

} // namespace
