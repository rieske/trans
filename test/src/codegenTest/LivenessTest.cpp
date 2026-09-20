#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "codegen/IrBuilders.h"
#include "codegen/IrStringTable.h"
#include "codegen/Liveness.h"

#include <string_view>

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

Procedure makeProc(IrStringTable& strings, std::vector<Instruction> body) {
    Procedure p;
    p.name = strings.intern("f");
    p.body = std::move(body);
    internProcedureTemps(strings, p);
    return p;
}

TEST(Liveness, condJumpDiamondLiveInIsTargetUsesNotFallthroughTemp) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::add(n("a"), n("a"), n("t")),
            ir::jump(n("else"), JumpCondition::IF_EQUAL),
            ir::ret(n("t")),
            ir::label(n("else")),
            ir::ret(n("a")),
    }));

    ASSERT_THAT(live.atLabel.count(n("else")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("else")), UnorderedElementsAre(n("a")));
}

TEST(Liveness, loopHeaderKeepsIncTarget) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::label(n("L")),
            ir::inc(n("x")),
            ir::jump(n("L")),
    }));

    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), UnorderedElementsAre(n("x")));
}

TEST(Liveness, afterInstKeepsDefLiveOnBackEdge) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("t")),
            ir::label(n("L")),
            ir::assign(n("t"), n("r")),
            ir::assignConstant(n("1"), n("t")),
            ir::jump(n("L")),
    });
    const ProcedureLiveness live = computeProcedureLiveness(p);
    ASSERT_THAT(live.afterInst.size(), Eq(p.body.size()));
    const int def = 3;
    ASSERT_THAT(p.body[static_cast<std::size_t>(def)].op, Eq(Op::AssignConstant));
    EXPECT_THAT(live.afterInst[static_cast<std::size_t>(def)], Contains(n("t")));
}

TEST(Liveness, argumentStaysLiveAcrossLabelBeforeCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::argument(n("a")),
            ir::label(n("L")),
            ir::call(n("foo")),
            ir::voidReturn(),
    }));

    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), UnorderedElementsAre(n("a")));
}

TEST(Liveness, emptyLabelInheritsSuccessorLiveIn) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::label(n("L1")),
            ir::label(n("L2")),
            ir::ret(n("x")),
    }));

    ASSERT_THAT(live.atLabel.count(n("L1")), Eq(1u));
    ASSERT_THAT(live.atLabel.count(n("L2")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L1")), UnorderedElementsAre(n("x")));
    EXPECT_THAT(live.atLabel.at(n("L2")), UnorderedElementsAre(n("x")));
}

TEST(Liveness, unreachableLabelStillHasAPresentSet) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::ret(n("x")),
            ir::label(n("dead")),
            ir::ret(n("y")),
    }));

    ASSERT_THAT(live.atLabel.count(n("dead")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("dead")), UnorderedElementsAre(n("y")));
}

TEST(Liveness, emptyLabeledBlockInsertsEmptySet) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::label(n("L")),
            ir::voidReturn(),
    }));

    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), IsEmpty());
}

TEST(Liveness, addressTakenIsLiveInAtEveryLabeledBlock) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::addressOf(n("a"), n("p")),
            ir::label(n("L")),
            ir::voidReturn(),
    }));

    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), UnorderedElementsAre(n("a")));
}

TEST(Liveness, addressTakenUnionsIntoInheritedLiveIn) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::addressOf(n("a"), n("p")),
            ir::label(n("L1")),
            ir::label(n("L2")),
            ir::ret(n("x")),
    }));

    ASSERT_THAT(live.atLabel.count(n("L1")), Eq(1u));
    ASSERT_THAT(live.atLabel.count(n("L2")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L1")), UnorderedElementsAre(n("x"), n("a")));
    EXPECT_THAT(live.atLabel.at(n("L2")), UnorderedElementsAre(n("x"), n("a")));
}

TEST(Liveness, addressTakenIsLiveInAtJoinAfterInc) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::addressOf(n("a"), n("p")),
            ir::jump(n("join"), JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::label(n("join")),
            ir::dereference(n("p"), n("r")),
            ir::ret(n("r")),
    }));

    ASSERT_THAT(live.atLabel.count(n("join")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("join")), UnorderedElementsAre(n("p"), n("a")));
}

TEST(Liveness, liveAfterCallKeepsArgUsedAfterNotSiblings) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const auto after = computeLiveAfterCalls(makeProc(ir.strings, {
            ir::argument(n("a")),
            ir::argument(n("b")),
            ir::argument(n("c")),
            ir::call(n("foo")),
            ir::ret(n("a")),
    }));

    ASSERT_THAT(after.count(3), Eq(1u));
    EXPECT_THAT(after.at(3), UnorderedElementsAre(n("a")));
}

TEST(Liveness, threePendingArgsStayLiveAtLabelBeforeCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const LabelLiveIns live = computeLabelLiveIns(makeProc(ir.strings, {
            ir::argument(n("a")),
            ir::argument(n("b")),
            ir::argument(n("c")),
            ir::label(n("L")),
            ir::call(n("foo")),
            ir::voidReturn(),
    }));

    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), UnorderedElementsAre(n("a"), n("b"), n("c")));
}

TEST(Liveness, liveAfterCallExcludesDeadTemp) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const auto after = computeLiveAfterCalls(makeProc(ir.strings, {
            ir::add(n("a"), n("a"), n("t")),
            ir::call(n("foo")),
            ir::ret(n("a")),
    }));

    ASSERT_THAT(after.count(1), Eq(1u));
    EXPECT_THAT(after.at(1), UnorderedElementsAre(n("a")));
}

TEST(Liveness, liveAfterCallIncludesTempUsedLater) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const auto after = computeLiveAfterCalls(makeProc(ir.strings, {
            ir::add(n("a"), n("a"), n("t")),
            ir::call(n("foo")),
            ir::ret(n("t")),
    }));

    ASSERT_THAT(after.count(1), Eq(1u));
    EXPECT_THAT(after.at(1), UnorderedElementsAre(n("t")));
}

TEST(Liveness, liveAfterCallKeepsValueUsedBeforeCallInLoop) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const auto after = computeLiveAfterCalls(makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("a"), n("t")),
            ir::call(n("foo")),
            ir::jump(n("L")),
    }));

    ASSERT_THAT(after.count(2), Eq(1u));
    EXPECT_THAT(after.at(2), UnorderedElementsAre(n("a")));
}

TEST(Liveness, procedureLivenessEmptyBodyHasEmptySets) {
    IntermediateRepresentation ir;
    const ProcedureLiveness live = computeProcedureLiveness(makeProc(ir.strings, {}));

    EXPECT_THAT(live.atLabel, IsEmpty());
    EXPECT_THAT(live.afterCall, IsEmpty());
    EXPECT_THAT(live.addressTaken, IsEmpty());
}

TEST(Liveness, procedureLivenessLoneCallHasEmptyAfterCallSet) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const ProcedureLiveness live = computeProcedureLiveness(makeProc(ir.strings, {
            ir::call(n("foo")),
    }));

    EXPECT_THAT(live.afterCall.size(), Eq(1u));
    ASSERT_THAT(live.afterCall.count(0), Eq(1u));
    EXPECT_THAT(live.afterCall.at(0), IsEmpty());
}

TEST(Liveness, procedureLivenessNoCallLeavesAfterCallEmpty) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const ProcedureLiveness live = computeProcedureLiveness(makeProc(ir.strings, {
            ir::label(n("L")),
            ir::ret(n("x")),
    }));

    EXPECT_THAT(live.afterCall, IsEmpty());
    EXPECT_THAT(live.addressTaken, IsEmpty());
    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), UnorderedElementsAre(n("x")));
}

TEST(Liveness, procedureLivenessCallKeepsTempAndAddressTaken) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    const ProcedureLiveness live = computeProcedureLiveness(makeProc(ir.strings, {
            ir::addressOf(n("a"), n("p")),
            ir::add(n("a"), n("a"), n("t")),
            ir::call(n("foo")),
            ir::label(n("L")),
            ir::ret(n("t")),
    }));

    EXPECT_THAT(live.addressTaken, UnorderedElementsAre(n("a")));
    ASSERT_THAT(live.atLabel.count(n("L")), Eq(1u));
    EXPECT_THAT(live.atLabel.at(n("L")), UnorderedElementsAre(n("t"), n("a")));
    EXPECT_THAT(live.afterCall.size(), Eq(1u));
    ASSERT_THAT(live.afterCall.count(2), Eq(1u));
    EXPECT_THAT(live.afterCall.at(2), UnorderedElementsAre(n("t")));
}

} // namespace
