#include "gtest/gtest.h"

#include <map>
#include <memory>
#include <vector>

#include "codegen/CodeGeneratingVisitor.h"
#include "codegen/FrameLayout.h"
#include "codegen/quadruples/AddressOf.h"
#include "codegen/quadruples/Argument.h"
#include "codegen/quadruples/Assign.h"
#include "codegen/quadruples/AssignConstant.h"
#include "codegen/quadruples/BasicBlock.h"
#include "codegen/quadruples/Call.h"
#include "codegen/quadruples/Retrieve.h"
#include "semantic_analyzer/ValueEntry.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace {

using namespace codegen;

translation_unit::Context ctx() {
    return translation_unit::Context { "test", 1 };
}

// Expression temps must match isExpressionTempName: "$t" + digits only.
// Multi-word array temp whose address is taken, then later scalar temps appear
// before the pointer is passed to a call. The array must not share a spill slot
// with those scalars (compound-literal call-arg / sizeof interleaving).
TEST(FrameLayout, addressOfExtendsMultiWordObjectThroughPointerUse) {
    // $t1 : int[3] (12 bytes, 2 words)
    // $t2 := &$t1
    // $t3 := 4          // would reclaim $t1 storage without liveness extend
    // PARAM $t2
    // CALL foo
    std::map<std::string, semantic_analyzer::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3);
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1",
            semantic_analyzer::ValueEntry { "$t1", arrTy, true, ctx(), 1, false });
    locals.emplace("$t2",
            semantic_analyzer::ValueEntry { "$t2", ptrTy, true, ctx(), 2, false });
    locals.emplace("$t3",
            semantic_analyzer::ValueEntry { "$t3", intTy, true, ctx(), 3, false });

    std::vector<std::unique_ptr<Quadruple>> bodyQuads;
    bodyQuads.push_back(std::make_unique<AddressOf>("$t1", "$t2"));
    bodyQuads.push_back(std::make_unique<AssignConstant>("4", "$t3"));
    bodyQuads.push_back(std::make_unique<Argument>("$t2"));
    bodyQuads.push_back(std::make_unique<Call>("foo", false, ""));

    auto blocks = toBasicBlocks(std::move(bodyQuads));
    std::vector<Value> values = packFrameValues(locals, blocks);

    int arrSlot = -1;
    int arrWords = 0;
    int szSlot = -1;
    int ptrSlot = -1;
    for (const auto& v : values) {
        if (v.getName() == "$t1") {
            arrSlot = v.getIndex();
            arrWords = (v.getSizeInBytes() + 7) / 8;
            if (arrWords < 1) {
                arrWords = 1;
            }
        } else if (v.getName() == "$t3") {
            szSlot = v.getIndex();
        } else if (v.getName() == "$t2") {
            ptrSlot = v.getIndex();
        }
    }
    ASSERT_GE(arrSlot, 0);
    ASSERT_GE(szSlot, 0);
    ASSERT_GE(ptrSlot, 0);
    // $t3 must not sit inside $t1's word span.
    const int arrEnd = arrSlot + arrWords;
    EXPECT_TRUE(szSlot < arrSlot || szSlot >= arrEnd)
            << "sizeof-like temp reclaimed array storage: arrSlot=" << arrSlot
            << " words=" << arrWords << " szSlot=" << szSlot;
}

// PARAM→CALL must *extend* live ranges, never shrink them. If a pointer is used
// again after the CALL, its last-use (and address-of base storage) must stay at
// that post-call mention. A buggy noteLive that overwrites last:=callIndex
// would let a mid temp reclaim multi-word object storage while the pointer is
// still live.
//
//   0: $t2 := &$t1
//   1: PARAM $t2
//   2: CALL foo
//   3: $t3 := 0          // between CALL and post-call pointer use
//   4: $t4 := $t2        // pointer still live after CALL
TEST(FrameLayout, paramCallExtendDoesNotShrinkPostCallPointerUse) {
    std::map<std::string, semantic_analyzer::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3); // 12 bytes, 2 words
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1",
            semantic_analyzer::ValueEntry { "$t1", arrTy, true, ctx(), 1, false });
    locals.emplace("$t2",
            semantic_analyzer::ValueEntry { "$t2", ptrTy, true, ctx(), 2, false });
    locals.emplace("$t3",
            semantic_analyzer::ValueEntry { "$t3", intTy, true, ctx(), 3, false });
    locals.emplace("$t4",
            semantic_analyzer::ValueEntry { "$t4", ptrTy, true, ctx(), 4, false });

    std::vector<std::unique_ptr<Quadruple>> bodyQuads;
    bodyQuads.push_back(std::make_unique<AddressOf>("$t1", "$t2"));      // 0
    bodyQuads.push_back(std::make_unique<Argument>("$t2"));               // 1
    bodyQuads.push_back(std::make_unique<Call>("foo", false, ""));        // 2
    bodyQuads.push_back(std::make_unique<AssignConstant>("0", "$t3"));    // 3
    bodyQuads.push_back(std::make_unique<Assign>("$t2", "$t4"));          // 4

    auto blocks = toBasicBlocks(std::move(bodyQuads));
    std::vector<Value> values = packFrameValues(locals, blocks);

    int arrSlot = -1;
    int arrWords = 0;
    int arrLast = -1;
    int ptrLast = -1;
    int midSlot = -1;
    for (const auto& v : values) {
        if (v.getName() == "$t1") {
            arrSlot = v.getIndex();
            arrWords = (v.getSizeInBytes() + 7) / 8;
            if (arrWords < 1) {
                arrWords = 1;
            }
            arrLast = v.getLastUseOrdinal();
        } else if (v.getName() == "$t2") {
            ptrLast = v.getLastUseOrdinal();
        } else if (v.getName() == "$t3") {
            midSlot = v.getIndex();
        }
    }
    ASSERT_GE(arrSlot, 0);
    ASSERT_GE(midSlot, 0);
    // Post-call Assign keeps $t2 live through ordinal 4; PARAM→CALL must not
    // pull last back to the CALL at 2.
    EXPECT_EQ(ptrLast, 4) << "PARAM→CALL shrank pointer last-use past post-call use";
    // Address-of base tracks the pointer: still live through ordinal 4.
    EXPECT_EQ(arrLast, 4) << "address-of base last-use did not track post-call pointer";
    // $t3 (ordinal 3) overlaps the object live range [0,4] — must not reuse its words.
    const int arrEnd = arrSlot + arrWords;
    EXPECT_TRUE(midSlot < arrSlot || midSlot >= arrEnd)
            << "mid temp reclaimed array storage while pointer still live: arrSlot="
            << arrSlot << " words=" << arrWords << " midSlot=" << midSlot;
}

} // namespace
