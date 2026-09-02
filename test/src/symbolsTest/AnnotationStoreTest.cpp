#include "gtest/gtest.h"

#include <stdexcept>

#include "symbols/AnnotationStore.h"
#include "symbols/FunctionEntry.h"
#include "symbols/FunctionFrame.h"
#include "symbols/LabelEntry.h"
#include "symbols/ValueEntry.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

#include <map>

namespace {

TEST(AnnotationStore, addressPlanRoundTrip) {
    symbols::AnnotationStore store;
    int node = 1;
    symbols::FieldPlan field;
    field.fieldOffsetBytes = 8;
    store.setAddressPlan(&node, symbols::AddressPlan { field });

    const auto* plan = store.addressPlan(&node);
    ASSERT_NE(plan, nullptr);
    const auto* f = symbols::get_if<symbols::FieldPlan>(plan);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->fieldOffsetBytes, 8);
    EXPECT_EQ(store.addressPlan(&node + 1), nullptr);
}

TEST(AnnotationStore, callPlanRoundTrip) {
    symbols::AnnotationStore store;
    int node = 2;
    store.setCallPlan(&node, symbols::DirectCallPlan { "printf" });

    const auto* got = store.callPlan(&node);
    ASSERT_NE(got, nullptr);
    EXPECT_FALSE(symbols::isIndirectCall(*got));
    EXPECT_EQ(symbols::callCalleeName(*got), "printf");
    const auto* direct = symbols::get_if<symbols::DirectCallPlan>(got);
    ASSERT_NE(direct, nullptr);
    EXPECT_EQ(direct->calleeName, "printf");
    EXPECT_EQ(store.callPlan(&node + 1), nullptr);
}

TEST(AnnotationStore, callCalleeNameRejectsVaBuiltin) {
    symbols::CallPlan plan { symbols::VaStartPlan {} };
    EXPECT_THROW(symbols::callCalleeName(plan), std::logic_error);
    EXPECT_FALSE(symbols::isIndirectCall(plan));
}

TEST(AnnotationStore, callPlanIndirect) {
    symbols::AnnotationStore store;
    int node = 6;
    store.setCallPlan(&node, symbols::IndirectCallPlan { "fp" });

    const auto* got = store.callPlan(&node);
    ASSERT_NE(got, nullptr);
    EXPECT_TRUE(symbols::isIndirectCall(*got));
    EXPECT_EQ(symbols::callCalleeName(*got), "fp");
    const auto* indirect = symbols::get_if<symbols::IndirectCallPlan>(got);
    ASSERT_NE(indirect, nullptr);
    EXPECT_EQ(indirect->calleeName, "fp");
}

TEST(AnnotationStore, structFieldInits) {
    symbols::AnnotationStore store;
    int node = 3;
    symbols::StructFieldInit a;
    a.offsetBytes = 0;
    a.addressName = "a0";
    a.sourceName = "s0";
    a.zeroInitialize = false;
    symbols::StructFieldInit b;
    b.offsetBytes = 4;
    b.addressName = "a1";
    b.sourceName = "z1";
    b.zeroInitialize = true;
    std::vector<symbols::StructFieldInit> first;
    first.push_back(std::move(a));
    first.push_back(std::move(b));
    store.setStructFieldInits(&node, std::move(first));

    const auto& inits = store.structFieldInits(&node);
    ASSERT_EQ(inits.size(), 2u);
    EXPECT_EQ(inits[0].offsetBytes, 0);
    EXPECT_FALSE(inits[0].zeroInitialize);
    EXPECT_EQ(inits[1].offsetBytes, 4);
    EXPECT_TRUE(inits[1].zeroInitialize);

    EXPECT_TRUE(store.structFieldInits(&node + 1).empty());

    std::vector<symbols::StructFieldInit> replaced;
    symbols::StructFieldInit c;
    c.offsetBytes = 8;
    c.addressName = "a2";
    c.sourceName = "s2";
    replaced.push_back(std::move(c));
    store.setStructFieldInits(&node, std::move(replaced));
    EXPECT_EQ(store.structFieldInits(&node).size(), 1u);
    EXPECT_EQ(store.structFieldInits(&node)[0].offsetBytes, 8);
}

TEST(AnnotationStore, resultSlotRoundTrip) {
    symbols::AnnotationStore store;
    int node = 7;
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry v("tmp", type::signedInteger(), ctx, 0);
    store.setResult(&node, v);
    ASSERT_TRUE(store.hasResult(&node));
    EXPECT_EQ(store.result(&node)->getName(), "tmp");
    const symbols::AnnotationStore& cstore = store;
    EXPECT_EQ(cstore.result(&node)->getName(), "tmp");
    EXPECT_FALSE(store.hasResult(&node + 1));
    EXPECT_EQ(store.value(&node + 1, symbols::ValueSlot::Result), nullptr);
    EXPECT_EQ(cstore.value(&node + 1, symbols::ValueSlot::Result), nullptr);
    // Node exists but slot empty.
    store.setAddressPlan(&node, symbols::AddressPlan { symbols::IndexPlan {} });
    EXPECT_EQ(store.value(&node, symbols::ValueSlot::Lvalue), nullptr);
    EXPECT_EQ(cstore.value(&node, symbols::ValueSlot::Lvalue), nullptr);
}

TEST(AnnotationStore, lvalueSlot) {
    symbols::AnnotationStore store;
    int node = 8;
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry lv("lv", type::pointer(type::signedInteger()), ctx, 1);
    store.setLvalue(&node, lv);
    ASSERT_NE(store.lvalue(&node), nullptr);
    EXPECT_EQ(store.lvalue(&node)->getName(), "lv");
    const symbols::AnnotationStore& cstore = store;
    EXPECT_EQ(cstore.lvalue(&node)->getName(), "lv");
}

TEST(AnnotationStore, functionFrameRoundTrip) {
    symbols::AnnotationStore store;
    int node = 10;
    translation_unit::Context ctx { "t.c", 1 };
    type::Type fnType = type::function(type::signedInteger(), { type::signedInteger() });
    symbols::FunctionEntry symbol { "add", fnType.getFunction(), ctx };
    symbols::ValueEntry local { "L$loc1_x", type::signedInteger(), ctx, 0 };
    symbols::ValueEntry arg { "L$loc1_a", type::signedInteger(), ctx, 0 };
    std::map<std::string, symbols::ValueEntry> locals;
    locals.emplace(local.getName(), local);
    store.setFunctionFrame(&node, symbols::FunctionFrame {
            std::move(symbol), std::move(locals), { arg } });

    const auto* got = store.functionFrame(&node);
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->symbol.getName(), "add");
    ASSERT_EQ(got->locals.size(), 1u);
    EXPECT_EQ(got->locals.begin()->first, "L$loc1_x");
    ASSERT_EQ(got->arguments.size(), 1u);
    EXPECT_EQ(got->arguments[0].getName(), "L$loc1_a");
    EXPECT_EQ(store.functionFrame(&node + 1), nullptr);
}

TEST(AnnotationStore, rodataLabelRoundTrip) {
    symbols::AnnotationStore store;
    int node = 11;
    store.setRodataLabel(&node, "L$str0");
    const auto* label = store.rodataLabel(&node);
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(*label, "L$str0");
    EXPECT_EQ(store.rodataLabel(&node + 1), nullptr);
}

TEST(AnnotationStore, sizeofValueRoundTrip) {
    symbols::AnnotationStore store;
    int node = 12;
    store.setSizeofValue(&node, 8);
    const auto* bytes = store.sizeofValue(&node);
    ASSERT_NE(bytes, nullptr);
    EXPECT_EQ(*bytes, 8);
    EXPECT_EQ(store.sizeofValue(&node + 1), nullptr);
}

TEST(AnnotationStore, clearEmptiesAll) {
    symbols::AnnotationStore store;
    int node = 4;
    translation_unit::Context ctx { "t", 1 };
    store.setCallPlan(&node, symbols::DirectCallPlan { "f" });
    store.setResult(&node, symbols::ValueEntry("r", type::signedInteger(), ctx, 0));
    store.setLvalue(&node, symbols::ValueEntry("lv", type::pointer(type::signedInteger()), ctx, 1));
    store.setLabel(&node, symbols::LabelSlot::Exit, symbols::LabelEntry { "Lx" });
    type::Type fnType = type::function(type::signedInteger(), {});
    store.setFunctionFrame(&node, symbols::FunctionFrame {
            symbols::FunctionEntry { "f", fnType.getFunction(), ctx }, {}, {} });
    store.setRodataLabel(&node, "L$str1");
    store.setSizeofValue(&node, 4);
    store.clear();
    EXPECT_EQ(store.callPlan(&node), nullptr);
    EXPECT_FALSE(store.hasResult(&node));
    EXPECT_EQ(store.lvalue(&node), nullptr);
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Exit), nullptr);
    EXPECT_EQ(store.functionFrame(&node), nullptr);
    EXPECT_EQ(store.rodataLabel(&node), nullptr);
    EXPECT_EQ(store.sizeofValue(&node), nullptr);
}

TEST(AnnotationStore, indexPlanVariant) {
    symbols::AnnotationStore store;
    int node = 5;
    symbols::IndexPlan idx;
    idx.elementSize = 4;
    idx.elementType = type::signedInteger();
    idx.baseMode = symbols::AddressBaseMode::LeaObject;
    store.setAddressPlan(&node, symbols::AddressPlan { idx });
    const auto* plan = store.addressPlan(&node);
    ASSERT_NE(plan, nullptr);
    const auto* i = symbols::get_if<symbols::IndexPlan>(plan);
    ASSERT_NE(i, nullptr);
    EXPECT_EQ(i->elementSize, 4);
    EXPECT_TRUE(i->elementType.equivalentTo(type::signedInteger()));
    EXPECT_EQ(i->baseMode, symbols::AddressBaseMode::LeaObject);
    EXPECT_TRUE(symbols::addressBaseUsesLea(i->baseMode));
}

TEST(AnnotationStore, labelSlots) {
    symbols::AnnotationStore store;
    int node = 0;
    store.setLabel(&node, symbols::LabelSlot::Falsy, symbols::LabelEntry { "Lf" });
    store.setLabel(&node, symbols::LabelSlot::Exit, symbols::LabelEntry { "Le" });
    ASSERT_NE(store.label(&node, symbols::LabelSlot::Falsy), nullptr);
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Falsy)->getName(), "Lf");
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Exit)->getName(), "Le");
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Truthy), nullptr);
    // Overwrite same slot.
    store.setLabel(&node, symbols::LabelSlot::Falsy, symbols::LabelEntry { "Lf2" });
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Falsy)->getName(), "Lf2");
    // Const access.
    const symbols::AnnotationStore& cstore = store;
    EXPECT_EQ(cstore.label(&node, symbols::LabelSlot::Exit)->getName(), "Le");
}


TEST(AnnotationStore, caseTempPreOperationAndHolderSlots) {
    symbols::AnnotationStore store;
    int node = 9;
    translation_unit::Context ctx { "t", 1 };
    store.setCaseTemp(&node, symbols::ValueEntry("ct", type::signedInteger(), ctx, 0));
    store.setPreOperation(&node, symbols::ValueEntry("pre", type::signedInteger(), ctx, 1));
    store.setHolder(&node, symbols::ValueEntry("hold", type::signedInteger(), ctx, 2));
    ASSERT_NE(store.caseTemp(&node), nullptr);
    EXPECT_EQ(store.caseTemp(&node)->getName(), "ct");
    ASSERT_NE(store.preOperation(&node), nullptr);
    EXPECT_EQ(store.preOperation(&node)->getName(), "pre");
    ASSERT_NE(store.holder(&node), nullptr);
    EXPECT_EQ(store.holder(&node)->getName(), "hold");
}

} // namespace
