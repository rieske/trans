#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "codegen/IrBuilders.h"
#include "codegen/IrInline.h"
#include "codegen/IrStringTable.h"
#include "codegen/Value.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
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

TEST(IrInline, callIsEligibleDoesNotTreatEmptyAsUnsafe) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "die", { ir::voidReturn() });
    EXPECT_FALSE(calleeLooksUnsafe(callee, ir.strings));
    EXPECT_TRUE(callIsEligible(ir::call(n("die")), caller, callee, ir.strings, {}, 0, 0, 0, true));
}

TEST(IrInline, inlineProceduresRefusesEmptyCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "die", { ir::voidReturn() }));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::call(n("die")),
            ir::assignConstant(n("1"), n("r")),
            ir::ret(n("r")),
    }));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    bool fCallsDie = false;
    for (const auto& inst : ir.procedures.back().body) {
        if (inst.op == Op::Call && inst.arg0 == n("die")) {
            fCallsDie = true;
        }
    }
    EXPECT_TRUE(fCallsDie);
}

TEST(IrInline, callIsEligibleAcceptsIdentityReturn) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "id", {
            ir::ret(n("x")),
    }, oneFormal(ir.strings, "x"));
    EXPECT_TRUE(callIsEligible(ir::call(n("id")), caller, callee, ir.strings, {}, 1, 0, 0, true));
}

TEST(IrInline, inlineProceduresRefusesWrapperAroundEmptyDie) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "die", { ir::voidReturn() }));
    ir.procedures.push_back(makeProc(ir.strings, "wrap", {
            ir::call(n("die")),
            ir::voidReturn(),
    }));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::call(n("wrap")),
            ir::assignConstant(n("1"), n("r")),
            ir::ret(n("r")),
    }));
    inlineProcedures(ir);
    bool fCallsWrap = false;
    for (const auto& procedure : ir.procedures) {
        if (procedure.name != n("f")) {
            continue;
        }
        for (const auto& inst : procedure.body) {
            if (inst.op == Op::Call && inst.arg0 == n("wrap")) {
                fCallsWrap = true;
            }
        }
    }
    EXPECT_TRUE(fCallsWrap);
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

void expectJumpsToTrailingLabel(const std::vector<Instruction>& body) {
    ASSERT_FALSE(body.empty());
    ASSERT_THAT(body.back().op, Eq(Op::Label));
    const int cont = body.back().arg0;
    bool sawJump = false;
    for (const auto& inst : body) {
        if (inst.op == Op::Jump) {
            EXPECT_THAT(inst.arg0, Eq(cont));
            sawJump = true;
        }
    }
    EXPECT_TRUE(sawJump);
}

void expectSiteLocalRemap(const std::vector<Instruction>& body, const IrStringTable& strings, int site) {
    const std::string cName = "L$inl" + std::to_string(site) + "_c";
    const std::string tName = "L$inl" + std::to_string(site) + "_t";
    ASSERT_FALSE(body.empty());
    const int cont = body.back().arg0;
    bool sawConst = false;
    bool sawAdd = false;
    bool sawBodyLabel = false;
    for (const auto& inst : body) {
        if (inst.op == Op::AssignConstant) {
            EXPECT_THAT(strings.get(inst.result), Eq(cName));
            sawConst = true;
        }
        if (inst.op == Op::Add) {
            EXPECT_THAT(strings.get(inst.arg1), Eq(cName));
            EXPECT_THAT(strings.get(inst.result), Eq(tName));
            sawAdd = true;
        }
        if (inst.op == Op::Label && inst.arg0 != cont) {
            EXPECT_THAT(strings.get(inst.arg0).compare(0, 3, "__L"), Eq(0));
            sawBodyLabel = true;
        }
    }
    EXPECT_TRUE(sawConst);
    EXPECT_TRUE(sawAdd);
    EXPECT_TRUE(sawBodyLabel);
}

std::set<int> clonedTempsAndLabels(const std::vector<Instruction>& body, const IrStringTable& strings) {
    std::set<int> ids;
    for (const auto& inst : body) {
        for (int id : { inst.arg0, inst.arg1, inst.result }) {
            if (id == kNoSymbol) {
                continue;
            }
            const std::string& name = strings.get(id);
            if (name.compare(0, 2, "$t") == 0 || name.compare(0, 3, "__L") == 0
                    || name.compare(0, 5, "L$inl") == 0) {
                ids.insert(id);
            }
        }
    }
    return ids;
}

TEST(IrInline, formalCanShareActualAllowsSafeTemp) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    const Procedure callee = smallCallee(ir.strings, n);
    codegen::Value temp { n("t0"), 0, Type::INTEGRAL, 4 };
    temp.markExpressionTemp();
    caller.frame.locals.push_back(temp);
    EXPECT_TRUE(formalCanShareActual(callee, n("x"), n("t0"), caller));
}

TEST(IrInline, cloneSharesSafeTempActual) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    codegen::Value temp { n("t0"), 0, Type::INTEGRAL, 4 };
    temp.markExpressionTemp();
    caller.frame.locals.push_back(temp);
    const Procedure callee = smallCallee(ir.strings, n);
    const auto cloned = cloneCalleeBody(caller, callee, { n("t0") }, n("r"), kNoSymbol, 0, ir.strings);
    bool sawAdd = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Assign) {
            EXPECT_THAT(ir.strings.get(inst.result).compare(0, 5, "L$inl"), Ne(0));
        }
        if (inst.op == Op::Add) {
            EXPECT_THAT(inst.arg0, Eq(n("t0")));
            sawAdd = true;
        }
    }
    EXPECT_TRUE(sawAdd);
}

TEST(IrInline, formalCanShareActualRefusesTempWhenCalleeHasLabel) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    codegen::Value temp { n("t0"), 0, Type::INTEGRAL, 4 };
    temp.markExpressionTemp();
    caller.frame.locals.push_back(temp);
    Procedure callee = makeProc(ir.strings, "add1", {
            ir::label(n("L")),
            ir::add(n("x"), n("c"), n("t")),
            ir::ret(n("t")),
    }, oneFormal(ir.strings, "x"));
    EXPECT_FALSE(formalCanShareActual(callee, n("x"), n("t0"), caller));
}

TEST(IrInline, formalCanShareActualRefusesGlobalAlias) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "add1", {
            ir::assignConstant(n("0"), n("g")),
            ir::add(n("x"), n("c"), n("t")),
            ir::ret(n("t")),
    }, oneFormal(ir.strings, "x"));
    EXPECT_FALSE(formalCanShareActual(callee, n("x"), n("g"), caller));
}

TEST(IrInline, formalCanShareActualRefusesLvalueAssignWithNamedActual) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "add1", {
            ir::lvalueAssign(n("c"), n("gp")),
            ir::ret(n("x")),
    }, oneFormal(ir.strings, "x"));
    EXPECT_FALSE(formalCanShareActual(callee, n("x"), n("y"), caller));
}

TEST(IrInline, formalCanShareActualRefusesAddressTakenCallerTemp) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = makeProc(ir.strings, "f", {
            ir::addressOf(n("t0"), n("p")),
            ir::argument(n("t0")),
            ir::call(n("add1")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    });
    codegen::Value temp { n("t0"), 0, Type::INTEGRAL, 4 };
    temp.markExpressionTemp();
    caller.frame.locals.push_back(temp);
    Procedure callee = makeProc(ir.strings, "add1", {
            ir::lvalueAssign(n("c"), n("gp")),
            ir::ret(n("x")),
    }, oneFormal(ir.strings, "x"));
    EXPECT_FALSE(formalCanShareActual(callee, n("x"), n("t0"), caller));
}

TEST(IrInline, cloneProducesDistinctIdsForTwoSites) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    ProcedureFrame frame = oneFormal(ir.strings, "x");
    frame.locals.push_back(integral(ir.strings, "c"));
    frame.locals.push_back(integral(ir.strings, "t"));
    Procedure callee = makeProc(ir.strings, "add1", {
            ir::assignConstant(n("1"), n("c")),
            ir::add(n("x"), n("c"), n("t")),
            ir::label(n("L")),
            ir::ret(n("t")),
    }, std::move(frame));
    const auto first = cloneCalleeBody(caller, callee, { n("y") }, n("r"), kNoSymbol, 0, ir.strings);
    const auto second = cloneCalleeBody(caller, callee, { n("y") }, n("r"), kNoSymbol, 1, ir.strings);
    expectSiteLocalRemap(first, ir.strings, 0);
    expectSiteLocalRemap(second, ir.strings, 1);
    const auto a = clonedTempsAndLabels(first, ir.strings);
    const auto b = clonedTempsAndLabels(second, ir.strings);
    EXPECT_THAT(a, Not(IsEmpty()));
    EXPECT_THAT(b, Not(IsEmpty()));
    std::vector<int> overlap;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(overlap));
    EXPECT_THAT(overlap, IsEmpty());
}

TEST(IrInline, cloneKeepsTwoXSlotsDistinct) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    ProcedureFrame frame;
    frame.arguments.push_back(integral(ir.strings, "a"));
    frame.locals.push_back(integral(ir.strings, "L$loc1_x"));
    frame.locals.push_back(integral(ir.strings, "L$loc2_x"));
    Procedure callee = makeProc(ir.strings, "mix", {
            ir::add(n("L$loc1_x"), n("L$loc2_x"), n("t")),
            ir::ret(n("t")),
    }, std::move(frame));
    const auto cloned = cloneCalleeBody(caller, callee, { n("y") }, n("r"), kNoSymbol, 0, ir.strings);
    std::set<std::string> inl;
    for (const auto& local : caller.frame.locals) {
        const std::string& name = ir.strings.get(local.id());
        if (name.compare(0, 5, "L$inl") == 0) {
            inl.insert(name);
        }
    }
    EXPECT_THAT(inl, Contains(std::string("L$inl0_loc1_x")));
    EXPECT_THAT(inl, Contains(std::string("L$inl0_loc2_x")));
    const int loc1 = n("L$inl0_loc1_x");
    const int loc2 = n("L$inl0_loc2_x");
    EXPECT_THAT(loc1, Ne(n("L$loc1_x")));
    EXPECT_THAT(loc2, Ne(n("L$loc2_x")));
    bool sawAdd = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Add) {
            EXPECT_THAT(inst.arg0, Eq(loc1));
            EXPECT_THAT(inst.arg1, Eq(loc2));
            sawAdd = true;
        }
    }
    EXPECT_TRUE(sawAdd);
}

TEST(IrInline, cloneLeavesSharedStaticAndConstant) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "next", {
            ir::assignConstant(n("1"), n("c")),
            ir::add(n("L$st1_n"), n("c"), n("L$st1_n")),
            ir::ret(n("L$st1_n")),
    });
    const auto cloned = cloneCalleeBody(caller, callee, {}, n("r"), kNoSymbol, 0, ir.strings);
    bool sawStatic = false;
    bool sawOne = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Add && inst.arg0 == n("L$st1_n") && inst.result == n("L$st1_n")) {
            sawStatic = true;
        }
        if (inst.op == Op::AssignConstant && inst.arg0 == n("1")) {
            sawOne = true;
        }
    }
    EXPECT_TRUE(sawStatic);
    EXPECT_TRUE(sawOne);
}

TEST(IrInline, cloneRewritesReturnToAssignAndJump) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = smallCallee(ir.strings, n);
    const auto cloned = cloneCalleeBody(caller, callee, { n("y") }, n("r"), kNoSymbol, 0, ir.strings);
    bool sawAssignToR = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Assign && inst.result == n("r")) {
            sawAssignToR = true;
        }
        EXPECT_THAT(inst.op, Ne(Op::Return));
    }
    EXPECT_TRUE(sawAssignToR);
    expectJumpsToTrailingLabel(cloned);
}

TEST(IrInline, cloneDiscardsReturnWithoutRetrieve) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = smallCallee(ir.strings, n);
    const auto cloned = cloneCalleeBody(caller, callee, { n("y") }, kNoSymbol, kNoSymbol, 0, ir.strings);
    bool sawAssignToR = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Assign && inst.result == n("r")) {
            sawAssignToR = true;
        }
        EXPECT_THAT(inst.op, Ne(Op::Return));
    }
    EXPECT_FALSE(sawAssignToR);
    expectJumpsToTrailingLabel(cloned);
}

TEST(IrInline, cloneRewritesVoidReturnToJump) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "sink", {
            ir::inc(n("x")),
            ir::voidReturn(),
    }, oneFormal(ir.strings, "x"));
    const auto cloned = cloneCalleeBody(caller, callee, { n("y") }, n("r"), kNoSymbol, 0, ir.strings);
    ASSERT_FALSE(cloned.empty());
    EXPECT_THAT(cloned.front().op, Eq(Op::Assign));
    const int copied = cloned.front().result;
    bool sawAssignToR = false;
    bool sawInc = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Assign && inst.result == n("r")) {
            sawAssignToR = true;
        }
        if (inst.op == Op::Inc) {
            EXPECT_THAT(inst.arg0, Eq(copied));
            sawInc = true;
        }
        EXPECT_THAT(inst.op, Ne(Op::VoidReturn));
        EXPECT_THAT(inst.op, Ne(Op::Return));
    }
    EXPECT_TRUE(sawInc);
    EXPECT_FALSE(sawAssignToR);
    expectJumpsToTrailingLabel(cloned);
}

TEST(IrInline, cloneCopiesFailedShareFormal) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "add1", {
            ir::assignConstant(n("0"), n("g")),
            ir::add(n("x"), n("c"), n("t")),
            ir::ret(n("t")),
    }, oneFormal(ir.strings, "x"));
    const auto cloned = cloneCalleeBody(caller, callee, { n("g") }, n("r"), kNoSymbol, 0, ir.strings);
    ASSERT_FALSE(cloned.empty());
    EXPECT_THAT(cloned.front().op, Eq(Op::Assign));
    EXPECT_THAT(cloned.front().arg0, Eq(n("g")));
    const int copied = cloned.front().result;
    EXPECT_THAT(copied, Ne(n("g")));
    EXPECT_THAT(copied, Ne(n("x")));
    bool onFrame = false;
    for (const auto& local : caller.frame.locals) {
        if (local.id() == copied) {
            onFrame = true;
        }
    }
    EXPECT_TRUE(onFrame);
    bool exprTemp = true;
    for (const auto& local : caller.frame.locals) {
        if (local.id() == copied) {
            exprTemp = local.isExpressionTemp();
        }
    }
    EXPECT_FALSE(exprTemp);
    bool sawAdd = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Add) {
            EXPECT_THAT(inst.arg0, Eq(copied));
            sawAdd = true;
        }
    }
    EXPECT_TRUE(sawAdd);
}

TEST(IrInline, cloneRemapsIncDecRetrieveAndNestedCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    ProcedureFrame frame = oneFormal(ir.strings, "x");
    frame.locals.push_back(integral(ir.strings, "t"));
    Procedure callee = makeProc(ir.strings, "wrap", {
            ir::inc(n("t")),
            ir::dec(n("t")),
            ir::argument(n("t")),
            ir::call(n("printf")),
            ir::retrieve(n("t")),
            ir::ret(n("t")),
    }, std::move(frame));
    const auto cloned = cloneCalleeBody(caller, callee, { n("y") }, n("r"), kNoSymbol, 0, ir.strings);
    bool sawPrintf = false;
    bool sawInc = false;
    bool sawDec = false;
    bool sawRetrieve = false;
    bool sawArgument = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::Call) {
            EXPECT_THAT(ir.strings.get(inst.arg0), Eq("printf"));
            sawPrintf = true;
        }
        if (inst.op == Op::Inc) {
            EXPECT_THAT(ir.strings.get(inst.arg0).compare(0, 5, "L$inl"), Eq(0));
            sawInc = true;
        }
        if (inst.op == Op::Dec) {
            EXPECT_THAT(ir.strings.get(inst.arg0).compare(0, 5, "L$inl"), Eq(0));
            sawDec = true;
        }
        if (inst.op == Op::Retrieve) {
            EXPECT_THAT(ir.strings.get(inst.result).compare(0, 5, "L$inl"), Eq(0));
            sawRetrieve = true;
        }
        if (inst.op == Op::Argument) {
            EXPECT_THAT(ir.strings.get(inst.arg0).compare(0, 5, "L$inl"), Eq(0));
            sawArgument = true;
        }
    }
    EXPECT_TRUE(sawPrintf);
    EXPECT_TRUE(sawInc);
    EXPECT_TRUE(sawDec);
    EXPECT_TRUE(sawRetrieve);
    EXPECT_TRUE(sawArgument);
}

TEST(IrInline, cloneRemapsSpecialIdFields) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    ProcedureFrame frame;
    frame.locals.push_back(integral(ir.strings, "p"));
    frame.locals.push_back(integral(ir.strings, "q"));
    Procedure callee = makeProc(ir.strings, "wrap", {
            ir::label(n("L")),
            ir::assignLabelAddress(n("L"), n("p")),
            ir::assignLabelAddress(n("L$str0"), n("q")),
            ir::functionAddress(n("printf"), n("p")),
            ir::call(n("p"), true),
            ir::voidReturn(),
    }, std::move(frame));
    const auto cloned = cloneCalleeBody(caller, callee, {}, n("r"), kNoSymbol, 0, ir.strings);
    bool sawPrivateLabelAddr = false;
    bool sawSharedStrAddr = false;
    bool sawFnAddr = false;
    bool sawIndirect = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::AssignLabelAddress && inst.arg0 == n("L$str0")) {
            EXPECT_THAT(ir.strings.get(inst.result).compare(0, 5, "L$inl"), Eq(0));
            sawSharedStrAddr = true;
        }
        if (inst.op == Op::AssignLabelAddress && inst.arg0 != n("L$str0") && inst.arg0 != n("L")) {
            EXPECT_THAT(ir.strings.get(inst.arg0).compare(0, 3, "__L"), Eq(0));
            EXPECT_THAT(ir.strings.get(inst.result).compare(0, 5, "L$inl"), Eq(0));
            sawPrivateLabelAddr = true;
        }
        if (inst.op == Op::FunctionAddress) {
            EXPECT_THAT(ir.strings.get(inst.arg0), Eq("printf"));
            EXPECT_THAT(ir.strings.get(inst.result).compare(0, 5, "L$inl"), Eq(0));
            sawFnAddr = true;
        }
        if (inst.op == Op::Call && inst.callIndirect) {
            EXPECT_THAT(ir.strings.get(inst.arg0).compare(0, 5, "L$inl"), Eq(0));
            sawIndirect = true;
        }
        EXPECT_THAT(inst.arg0, Ne(n("L")));
    }
    EXPECT_TRUE(sawPrivateLabelAddr);
    EXPECT_TRUE(sawSharedStrAddr);
    EXPECT_TRUE(sawFnAddr);
    EXPECT_TRUE(sawIndirect);
}

TEST(IrInline, cloneMapsSretToProvidedDest) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "retagg", {
            ir::addressOf(n("__sret"), n("p")),
            ir::voidReturn(),
    });
    callee.memoryReturn = true;
    callee.sretId = n("__sret");
    const auto cloned = cloneCalleeBody(caller, callee, {}, kNoSymbol, n("dest"), 0, ir.strings);
    bool sawAddr = false;
    for (const auto& inst : cloned) {
        if (inst.op == Op::AddressOf) {
            EXPECT_THAT(inst.arg0, Eq(n("dest")));
            sawAddr = true;
        }
    }
    EXPECT_TRUE(sawAddr);
}

TEST(IrInline, cloneRemapsSretWhenDestMissing) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure caller = smallCaller(ir.strings, n);
    Procedure callee = makeProc(ir.strings, "retagg", {
            ir::addressOf(n("__sret"), n("p")),
            ir::voidReturn(),
    });
    callee.memoryReturn = true;
    callee.sretId = n("__sret");
    const auto first = cloneCalleeBody(caller, callee, {}, kNoSymbol, kNoSymbol, 0, ir.strings);
    const auto second = cloneCalleeBody(caller, callee, {}, kNoSymbol, kNoSymbol, 1, ir.strings);
    int firstSret = kNoSymbol;
    int secondSret = kNoSymbol;
    for (const auto& inst : first) {
        if (inst.op == Op::AddressOf) {
            firstSret = inst.arg0;
        }
    }
    for (const auto& inst : second) {
        if (inst.op == Op::AddressOf) {
            secondSret = inst.arg0;
        }
    }
    EXPECT_THAT(firstSret, Ne(kNoSymbol));
    EXPECT_THAT(secondSret, Ne(kNoSymbol));
    EXPECT_THAT(firstSret, Ne(n("__sret")));
    EXPECT_THAT(secondSret, Ne(n("__sret")));
    EXPECT_THAT(firstSret, Ne(secondSret));
}

TEST(IrInline, inlineProceduresRefusesIndirectCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(smallCallee(ir.strings, n));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::argument(n("y")),
            ir::call(n("add1")),
            ir::retrieve(n("t")),
            ir::argument(n("y")),
            ir::call(n("add1"), true),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    }, oneFormal(ir.strings, "y")));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Ge(1));
    bool sawIndirect = false;
    for (const auto& inst : ir.procedures.back().body) {
        if (inst.op == Op::Call && inst.callIndirect && inst.arg0 == n("add1")) {
            sawIndirect = true;
        }
    }
    EXPECT_TRUE(sawIndirect);
}

TEST(IrInline, inlineProceduresRefusesVariadicCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure callee = smallCallee(ir.strings, n);
    callee.variadic = true;
    ir.procedures.push_back(std::move(callee));
    ir.procedures.push_back(smallCaller(ir.strings, n));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    bool sawCall = false;
    for (const auto& inst : ir.procedures.back().body) {
        if (inst.op == Op::Call && inst.arg0 == n("add1")) {
            sawCall = true;
        }
    }
    EXPECT_TRUE(sawCall);
}

TEST(IrInline, inlineProceduresRefusesOversizeCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    std::vector<Instruction> body;
    body.reserve(33);
    for (int i = 0; i < 33; ++i) {
        body.push_back(ir::inc(n("x")));
    }
    ir.procedures.push_back(makeProc(ir.strings, "fat", std::move(body), oneFormal(ir.strings, "x")));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::argument(n("y")),
            ir::call(n("fat")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    }, oneFormal(ir.strings, "y")));
    EXPECT_THAT(nonLabelCount(ir.procedures.front().body), Eq(33));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    bool sawCall = false;
    for (const auto& inst : ir.procedures.back().body) {
        if (inst.op == Op::Call && inst.arg0 == n("fat")) {
            sawCall = true;
        }
    }
    EXPECT_TRUE(sawCall);
}

TEST(IrInline, inlineProceduresRefusesSigsetjmpCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "wrap", {
            ir::call(n("__sigsetjmp")),
            ir::voidReturn(),
    }));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::call(n("wrap")),
            ir::voidReturn(),
    }));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    bool sawCall = false;
    for (const auto& inst : ir.procedures.back().body) {
        if (inst.op == Op::Call && inst.arg0 == n("wrap")) {
            sawCall = true;
        }
    }
    EXPECT_TRUE(sawCall);
}

TEST(IrInline, inlineProceduresRefusesLongjmpChkCallee) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "wrap", {
            ir::call(n("__longjmp_chk")),
            ir::voidReturn(),
    }));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::call(n("wrap")),
            ir::voidReturn(),
    }));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    bool sawCall = false;
    for (const auto& inst : ir.procedures.back().body) {
        if (inst.op == Op::Call && inst.arg0 == n("wrap")) {
            sawCall = true;
        }
    }
    EXPECT_TRUE(sawCall);
}

TEST(IrInline, inlineProceduresLeavesExternalOnlyTuUntouched) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::argument(n("y")),
            ir::call(n("printf")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    }, oneFormal(ir.strings, "y")));
    const Instruction* before = ir.procedures.front().body.data();
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    EXPECT_THAT(ir.procedures.front().body.data(), Eq(before));
}

TEST(IrInline, inlineProceduresSplicesEligibleCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(smallCallee(ir.strings, n));
    ir.procedures.push_back(smallCaller(ir.strings, n));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(1));
    bool callerHasCall = false;
    bool calleeRemains = false;
    bool sawAdd = false;
    bool sawAssignToR = false;
    for (const auto& procedure : ir.procedures) {
        if (procedure.name == n("add1")) {
            calleeRemains = true;
        }
        if (procedure.name == n("f")) {
            for (const auto& inst : procedure.body) {
                if (inst.op == Op::Call) {
                    callerHasCall = true;
                }
                if (inst.op == Op::Add) {
                    sawAdd = true;
                }
                if (inst.op == Op::Assign && inst.result == n("r")) {
                    sawAssignToR = true;
                }
            }
        }
    }
    EXPECT_TRUE(calleeRemains);
    EXPECT_FALSE(callerHasCall);
    EXPECT_TRUE(sawAdd);
    EXPECT_TRUE(sawAssignToR);
}

TEST(IrInline, inlineProceduresKeepsSelfRecursiveCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "fact", {
            ir::argument(n("t")),
            ir::call(n("fact")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    }, oneFormal(ir.strings, "n")));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Eq(0));
    EXPECT_THAT(stats.refusedRecursion, Ge(1));
    bool sawCall = false;
    for (const auto& inst : ir.procedures.front().body) {
        if (inst.op == Op::Call && inst.arg0 == n("fact")) {
            sawCall = true;
        }
    }
    EXPECT_TRUE(sawCall);
}

TEST(IrInline, inlineProceduresKeepsMutualBackEdge) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "a", {
            ir::argument(n("x")),
            ir::call(n("b")),
            ir::retrieve(n("r")),
            ir::ret(n("r")),
    }, oneFormal(ir.strings, "x")));
    ir.procedures.push_back(makeProc(ir.strings, "b", {
            ir::argument(n("y")),
            ir::call(n("a")),
            ir::retrieve(n("s")),
            ir::ret(n("s")),
    }, oneFormal(ir.strings, "y")));
    const InlineStats stats = inlineProcedures(ir);
    EXPECT_THAT(stats.sitesInlined, Ge(1));
    bool sawBackEdge = false;
    for (const auto& procedure : ir.procedures) {
        for (const auto& inst : procedure.body) {
            if (inst.op == Op::Call && (inst.arg0 == n("a") || inst.arg0 == n("b"))) {
                sawBackEdge = true;
            }
        }
    }
    EXPECT_TRUE(sawBackEdge);
}

} // namespace
