#include "gtest/gtest.h"

#include <map>
#include <vector>

#include "codegen/FrameLayout.h"
#include "codegen/Instruction.h"
#include "codegen/IrPasses.h"
#include "symbols/ValueEntry.h"
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
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3);
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1",
            symbols::ValueEntry { "$t1", arrTy, true, ctx(), 1, false });
    locals.emplace("$t2",
            symbols::ValueEntry { "$t2", ptrTy, true, ctx(), 2, false });
    locals.emplace("$t3",
            symbols::ValueEntry { "$t3", intTy, true, ctx(), 3, false });

    std::vector<Instruction> body {
            ir::addressOf("$t1", "$t2"),
            ir::assignConstant("4", "$t3"),
            ir::argument("$t2"),
            ir::call("foo", false, ""),
    };

    std::vector<Value> values = packFrameValues(locals, body);

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
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3); // 12 bytes, 2 words
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1",
            symbols::ValueEntry { "$t1", arrTy, true, ctx(), 1, false });
    locals.emplace("$t2",
            symbols::ValueEntry { "$t2", ptrTy, true, ctx(), 2, false });
    locals.emplace("$t3",
            symbols::ValueEntry { "$t3", intTy, true, ctx(), 3, false });
    locals.emplace("$t4",
            symbols::ValueEntry { "$t4", ptrTy, true, ctx(), 4, false });

    std::vector<Instruction> body {
            ir::addressOf("$t1", "$t2"),      // 0
            ir::argument("$t2"),               // 1
            ir::call("foo", false, ""),        // 2
            ir::assignConstant("0", "$t3"),    // 3
            ir::assign("$t2", "$t4"),          // 4
    };

    std::vector<Value> values = packFrameValues(locals, body);

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

// Nested call after string pool lea: format temp must keep its slot across the
// inner CALL so arg temps of the nested call do not share its spill home.
//
//   0: $t1 := &L$str1          // format
//   1: $t2 := 5
//   2: $t3 := 2
//   3: PARAM $t2
//   4: PARAM $t3
//   5: CALL forSum
//   6: RETRIEVE $t4
//   7: PARAM $t1
//   8: PARAM $t4
//   9: CALL printf
TEST(FrameLayout, assignLabelAddressLiveAcrossNestedCall) {
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type intTy = type::signedInteger();
    type::Type ptrTy = type::pointer(type::signedCharacter());
    locals.emplace("$t1",
            symbols::ValueEntry { "$t1", ptrTy, true, ctx(), 1, false });
    locals.emplace("$t2",
            symbols::ValueEntry { "$t2", intTy, true, ctx(), 2, false });
    locals.emplace("$t3",
            symbols::ValueEntry { "$t3", intTy, true, ctx(), 3, false });
    locals.emplace("$t4",
            symbols::ValueEntry { "$t4", intTy, true, ctx(), 4, false });

    std::vector<Instruction> body {
            ir::assignLabelAddress("L$str1", "$t1"), // 0
            ir::assignConstant("5", "$t2"),           // 1
            ir::assignConstant("2", "$t3"),           // 2
            ir::argument("$t2"),                      // 3
            ir::argument("$t3"),                      // 4
            ir::call("forSum", false, ""),            // 5
            ir::retrieve("$t4", false),               // 6
            ir::argument("$t1"),                      // 7
            ir::argument("$t4"),                      // 8
            ir::call("printf", false, ""),            // 9
    };

    std::vector<Value> values = packFrameValues(locals, body);

    int fmtSlot = -1;
    int fmtLast = -1;
    int aSlot = -1;
    int bSlot = -1;
    for (const auto& v : values) {
        if (v.getName() == "$t1") {
            fmtSlot = v.getIndex();
            fmtLast = v.getLastUseOrdinal();
        } else if (v.getName() == "$t2") {
            aSlot = v.getIndex();
        } else if (v.getName() == "$t3") {
            bSlot = v.getIndex();
        }
    }
    ASSERT_GE(fmtSlot, 0);
    ASSERT_GE(aSlot, 0);
    ASSERT_GE(bSlot, 0);
    // Live from def (0) through outer CALL (9); PARAM->CALL extends last use.
    EXPECT_EQ(fmtLast, 9) << "format label address last-use must reach outer CALL";
    // Inner-call args are live [1/2,5]; they overlap the format range [0,9].
    EXPECT_NE(fmtSlot, aSlot) << "format slot collided with nested arg $t2";
    EXPECT_NE(fmtSlot, bSlot) << "format slot collided with nested arg $t3";
}

// Pack must run on the post-pass body: jump-to-next is deleted first, then
// lastUseOrdinal is an index into that shortened schedule.
TEST(FrameLayout, lastUseOrdinalsMatchBodyAfterRunIrPasses) {
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1",
            symbols::ValueEntry { "$t1", intTy, true, ctx(), 1, false });

    IntermediateRepresentation ir;
    Procedure p;
    p.name = "f";
    p.body = {
            ir::assignConstant("1", "$t1"),
            ir::jump("L"),
            ir::label("L"),
            ir::assign("$t1", "$t1"),
            ir::voidReturn(),
    };
    ir.procedures.push_back(std::move(p));
    ir = runIrPasses(std::move(ir));

    ASSERT_EQ(ir.procedures[0].body.size(), 4u);
    std::vector<Value> values = packFrameValues(locals, ir.procedures[0].body);
    int last = -1;
    for (const auto& v : values) {
        if (v.getName() == "$t1") {
            last = v.getLastUseOrdinal();
        }
    }
    EXPECT_EQ(last, 2) << "last use is the post-peephole assign, not the pre-pass jump index";
}

} // namespace
