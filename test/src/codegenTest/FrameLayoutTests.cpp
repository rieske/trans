#include "gtest/gtest.h"

#include <map>
#include <string>
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

symbols::ValueEntry makeTemp(const std::string& name, const type::Type& ty, int index) {
    symbols::ValueEntry e { name, ty, ctx(), index };
    e.markExpressionTemp();
    return e;
}

// Expression temps use ValueEntry::markExpressionTemp (product bit).
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
    locals.emplace("$t1", makeTemp("$t1", arrTy, 1));
    locals.emplace("$t2", makeTemp("$t2", ptrTy, 2));
    locals.emplace("$t3", makeTemp("$t3", intTy, 3));

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

// &arr[0] is Index/AddressOf into a pointer temp, then Assign to the & result.
// The object must stay live through that copy, not die at the Assign.
TEST(FrameLayout, assignCopyExtendsAddressOfObjectThroughDest) {
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3);
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1", makeTemp("$t1", arrTy, 1));
    locals.emplace("$t2", makeTemp("$t2", ptrTy, 2));
    locals.emplace("$t3", makeTemp("$t3", ptrTy, 3));
    locals.emplace("$t4", makeTemp("$t4", intTy, 4));

    std::vector<Instruction> body {
            ir::addressOf("$t1", "$t2"),
            ir::assign("$t2", "$t3"),
            ir::assignConstant("4", "$t4"),
            ir::argument("$t3"),
            ir::call("foo", false, ""),
    };

    std::vector<Value> values = packFrameValues(locals, body);
    int arrSlot = -1;
    int arrWords = 0;
    int midSlot = -1;
    int arrLast = -1;
    for (const auto& v : values) {
        if (v.getName() == "$t1") {
            arrSlot = v.getIndex();
            arrWords = (v.getSizeInBytes() + 7) / 8;
            if (arrWords < 1) {
                arrWords = 1;
            }
            arrLast = v.getLastUseOrdinal();
        } else if (v.getName() == "$t4") {
            midSlot = v.getIndex();
        }
    }
    ASSERT_GE(arrSlot, 0);
    ASSERT_GE(midSlot, 0);
    EXPECT_EQ(arrLast, 4) << "object must stay live through the pointer copy at CALL";
    const int arrEnd = arrSlot + arrWords;
    EXPECT_TRUE(midSlot < arrSlot || midSlot >= arrEnd)
            << "mid temp reclaimed array storage after pointer Assign: arrSlot="
            << arrSlot << " words=" << arrWords << " midSlot=" << midSlot;
}

// First-wins alias growth has no iteration cap: a long pointer-copy chain
// after AddressOf must still keep the object live through CALL.
TEST(FrameLayout, longAssignChainExtendsAddressOfObjectThroughDest) {
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3);
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$obj", makeTemp("$obj", arrTy, 1));
    locals.emplace("$mid", makeTemp("$mid", intTy, 2));
    std::vector<Instruction> body;
    body.push_back(ir::addressOf("$obj", "$p0"));
    locals.emplace("$p0", makeTemp("$p0", ptrTy, 10));
    const int copies = 10;
    for (int i = 0; i < copies; ++i) {
        const std::string src = "$p" + std::to_string(i);
        const std::string dest = "$p" + std::to_string(i + 1);
        locals.emplace(dest, makeTemp(dest, ptrTy, 11 + i));
        body.push_back(ir::assign(src, dest));
    }
    body.push_back(ir::assignConstant("4", "$mid"));
    body.push_back(ir::argument("$p" + std::to_string(copies)));
    body.push_back(ir::call("foo", false, ""));

    std::vector<Value> values = packFrameValues(locals, body);
    int arrSlot = -1;
    int arrWords = 0;
    int midSlot = -1;
    int arrLast = -1;
    for (const auto& v : values) {
        if (v.getName() == "$obj") {
            arrSlot = v.getIndex();
            arrWords = (v.getSizeInBytes() + 7) / 8;
            if (arrWords < 1) {
                arrWords = 1;
            }
            arrLast = v.getLastUseOrdinal();
        } else if (v.getName() == "$mid") {
            midSlot = v.getIndex();
        }
    }
    ASSERT_GE(arrSlot, 0);
    ASSERT_GE(midSlot, 0);
    const int callOrdinal = static_cast<int>(body.size()) - 1;
    EXPECT_EQ(arrLast, callOrdinal) << "object must follow a long Assign chain to CALL";
    const int arrEnd = arrSlot + arrWords;
    EXPECT_TRUE(midSlot < arrSlot || midSlot >= arrEnd)
            << "mid temp reclaimed array storage after long Assign chain";
}

TEST(FrameLayout, leaObjectFieldAddressExtendsMultiWordObjectThroughPointerUse) {
    std::map<std::string, symbols::ValueEntry> locals;
    type::Type arrTy = type::array(type::signedInteger(), 3);
    type::Type ptrTy = type::pointer(type::signedInteger());
    type::Type intTy = type::signedInteger();
    locals.emplace("$t1", makeTemp("$t1", arrTy, 1));
    locals.emplace("$t2", makeTemp("$t2", ptrTy, 2));
    locals.emplace("$t3", makeTemp("$t3", intTy, 3));

    std::vector<Instruction> body {
            ir::fieldAddress("$t1", 0, "$t2", symbols::AddressBaseMode::LeaObject),
            ir::assignConstant("4", "$t3"),
            ir::argument("$t2"),
            ir::call("foo", false, ""),
    };

    std::vector<Value> values = packFrameValues(locals, body);
    int arrSlot = -1;
    int arrWords = 0;
    int szSlot = -1;
    for (const auto& v : values) {
        if (v.getName() == "$t1") {
            arrSlot = v.getIndex();
            arrWords = (v.getSizeInBytes() + 7) / 8;
            if (arrWords < 1) {
                arrWords = 1;
            }
        } else if (v.getName() == "$t3") {
            szSlot = v.getIndex();
        }
    }
    ASSERT_GE(arrSlot, 0);
    ASSERT_GE(szSlot, 0);
    const int arrEnd = arrSlot + arrWords;
    EXPECT_TRUE(szSlot < arrSlot || szSlot >= arrEnd)
            << "fieldAddress LeaObject failed to keep $t1 live: arrSlot=" << arrSlot
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
    locals.emplace("$t1", makeTemp("$t1", arrTy, 1));
    locals.emplace("$t2", makeTemp("$t2", ptrTy, 2));
    locals.emplace("$t3", makeTemp("$t3", intTy, 3));
    locals.emplace("$t4", makeTemp("$t4", ptrTy, 4));

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
//   0: $t1 := &__str1          // format
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
    locals.emplace("$t1", makeTemp("$t1", ptrTy, 1));
    locals.emplace("$t2", makeTemp("$t2", intTy, 2));
    locals.emplace("$t3", makeTemp("$t3", intTy, 3));
    locals.emplace("$t4", makeTemp("$t4", intTy, 4));

    std::vector<Instruction> body {
            ir::assignLabelAddress("__str1", "$t1"), // 0
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
    locals.emplace("$t1", makeTemp("$t1", intTy, 1));

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
