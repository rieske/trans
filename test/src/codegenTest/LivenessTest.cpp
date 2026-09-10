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

} // namespace
