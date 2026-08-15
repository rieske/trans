#include "gtest/gtest.h"

#include <vector>

#include "codegen/FrameLayout.h"
#include "codegen/Instruction.h"
#include "codegen/IrPasses.h"
#include "codegen/Value.h"

namespace {

using namespace codegen;

Value makeTemp(int id, int sizeBytes) {
    Value v { id, 0, Type::INTEGRAL, sizeBytes };
    v.markExpressionTemp();
    return v;
}

Value makeNamed(int id, int sizeBytes) {
    return Value { id, 0, Type::INTEGRAL, sizeBytes };
}

int slotOf(const std::vector<Value>& values, int id) {
    for (const auto& v : values) {
        if (v.id() == id) {
            return v.getIndex();
        }
    }
    return -1;
}

int lastOf(const std::vector<Value>& values, int id) {
    for (const auto& v : values) {
        if (v.id() == id) {
            return v.getLastUseOrdinal();
        }
    }
    return -1;
}

bool disjointSlots(int a, int aWords, int b) {
    return b < a || b >= a + aWords;
}

TEST(FrameLayout, oneWordTempsReuseAfterLastUse) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");

    std::vector<Value> values = packFrameValues(
            { makeTemp(t1, 4), makeTemp(t2, 4) },
            {
                    ir::assignConstant(strings.intern("1"), t1),
                    ir::assignConstant(strings.intern("2"), t2),
            });
    EXPECT_EQ(slotOf(values, t1), slotOf(values, t2));
}

TEST(FrameLayout, overlappingOneWordTempsKeepDistinctSlots) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");

    std::vector<Value> values = packFrameValues(
            { makeTemp(t1, 4), makeTemp(t2, 4) },
            {
                    ir::assignConstant(strings.intern("1"), t1),
                    ir::assign(t1, t2),
            });
    EXPECT_NE(slotOf(values, t1), slotOf(values, t2));
}

TEST(FrameLayout, namedLocalIsNotReusedByLaterTemp) {
    IrStringTable strings;
    const int x = strings.intern("x");
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");

    std::vector<Value> values = packFrameValues(
            { makeNamed(x, 4), makeTemp(t1, 4), makeTemp(t2, 4) },
            {
                    ir::assignConstant(strings.intern("1"), x),
                    ir::assignConstant(strings.intern("2"), t1),
                    ir::assignConstant(strings.intern("3"), t2),
            });
    EXPECT_NE(slotOf(values, x), slotOf(values, t1));
    EXPECT_NE(slotOf(values, x), slotOf(values, t2));
}

TEST(FrameLayout, addressTakenTempIsPinnedAfterPointerDies) {
    IrStringTable strings;
    const int obj = strings.intern("$obj");
    const int ptr = strings.intern("$ptr");
    const int later = strings.intern("$later");
    const int foo = strings.intern("foo");

    std::vector<Value> values = packFrameValues(
            { makeTemp(obj, 4), makeTemp(ptr, 8), makeTemp(later, 4) },
            {
                    ir::addressOf(obj, ptr),
                    ir::argument(ptr),
                    ir::call(foo, false, kNoSymbol),
                    ir::assignConstant(strings.intern("0"), later),
            });
    EXPECT_TRUE(disjointSlots(slotOf(values, obj), 1, slotOf(values, later)))
            << "address-taken temp must stay pinned after the pointer dies";
}

TEST(FrameLayout, leaObjectPinsBaseAfterPointerDies) {
    IrStringTable strings;
    const int obj = strings.intern("$obj");
    const int ptr = strings.intern("$ptr");
    const int later = strings.intern("$later");
    const int foo = strings.intern("foo");

    std::vector<Value> values = packFrameValues(
            { makeTemp(obj, 4), makeTemp(ptr, 8), makeTemp(later, 4) },
            {
                    ir::fieldAddress(obj, 0, ptr, symbols::AddressBaseMode::LeaObject),
                    ir::argument(ptr),
                    ir::call(foo, false, kNoSymbol),
                    ir::assignConstant(strings.intern("0"), later),
            });
    EXPECT_TRUE(disjointSlots(slotOf(values, obj), 1, slotOf(values, later)));
}

TEST(FrameLayout, multiWordTempIsPinnedAfterLastUse) {
    IrStringTable strings;
    const int wide = strings.intern("$wide");
    const int later = strings.intern("$later");

    std::vector<Value> values = packFrameValues(
            { makeTemp(wide, 12), makeTemp(later, 4) },
            {
                    ir::assignConstant(strings.intern("1"), wide),
                    ir::assignConstant(strings.intern("2"), later),
            });
    EXPECT_TRUE(disjointSlots(slotOf(values, wide), 2, slotOf(values, later)))
            << "multi-word temp must stay pinned after its last mention";
}

TEST(FrameLayout, paramCallKeepsArgLiveAcrossCall) {
    IrStringTable strings;
    const int arg = strings.intern("$arg");
    const int mid = strings.intern("$mid");
    const int foo = strings.intern("foo");

    std::vector<Value> values = packFrameValues(
            { makeTemp(arg, 4), makeTemp(mid, 4) },
            {
                    ir::assignConstant(strings.intern("1"), arg),
                    ir::argument(arg),
                    ir::assignConstant(strings.intern("2"), mid),
                    ir::call(foo, false, kNoSymbol),
            });
    EXPECT_NE(slotOf(values, arg), slotOf(values, mid));
    EXPECT_EQ(lastOf(values, arg), 3);
}

TEST(FrameLayout, argumentWithoutCallDoesNotExtendPastArgument) {
    IrStringTable strings;
    const int arg = strings.intern("$arg");
    const int later = strings.intern("$later");

    std::vector<Value> values = packFrameValues(
            { makeTemp(arg, 4), makeTemp(later, 4) },
            {
                    ir::assignConstant(strings.intern("1"), arg),
                    ir::argument(arg),
                    ir::assignConstant(strings.intern("2"), later),
            });
    EXPECT_EQ(lastOf(values, arg), 1);
    EXPECT_EQ(slotOf(values, arg), slotOf(values, later));
}

TEST(FrameLayout, assignLabelAddressLiveAcrossNestedCall) {
    IrStringTable strings;
    const int fmt = strings.intern("$fmt");
    const int a = strings.intern("$a");
    const int b = strings.intern("$b");
    const int sum = strings.intern("$sum");
    const int str = strings.intern("__str1");
    const int forSum = strings.intern("forSum");
    const int printfId = strings.intern("printf");

    std::vector<Value> values = packFrameValues(
            { makeTemp(fmt, 8), makeTemp(a, 4), makeTemp(b, 4), makeTemp(sum, 4) },
            {
                    ir::assignLabelAddress(str, fmt),
                    ir::assignConstant(strings.intern("5"), a),
                    ir::assignConstant(strings.intern("2"), b),
                    ir::argument(a),
                    ir::argument(b),
                    ir::call(forSum, false, kNoSymbol),
                    ir::retrieve(sum, false),
                    ir::argument(fmt),
                    ir::argument(sum),
                    ir::call(printfId, false, kNoSymbol),
            });
    EXPECT_EQ(lastOf(values, fmt), 9);
    EXPECT_NE(slotOf(values, fmt), slotOf(values, a));
    EXPECT_NE(slotOf(values, fmt), slotOf(values, b));
}

TEST(FrameLayout, lastUseOrdinalsMatchBodyAfterRunIrPasses) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int L = strings.intern("L");

    IntermediateRepresentation ir;
    Procedure p;
    p.name = strings.intern("f");
    p.body = {
            ir::assignConstant(strings.intern("1"), t1),
            ir::jump(L),
            ir::label(L),
            ir::assign(t1, t1),
            ir::voidReturn(),
    };
    ir.procedures.push_back(std::move(p));
    ir = runIrPasses(std::move(ir));

    ASSERT_EQ(ir.procedures[0].body.size(), 4u);
    std::vector<Value> values = packFrameValues({ makeTemp(t1, 4) }, ir.procedures[0].body);
    EXPECT_EQ(lastOf(values, t1), 2);
}

} // namespace
