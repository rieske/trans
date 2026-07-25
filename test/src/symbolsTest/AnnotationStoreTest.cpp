#include "gtest/gtest.h"

#include "symbols/AnnotationStore.h"

namespace {

TEST(AnnotationStore, addressPlanRoundTrip) {
    symbols::AnnotationStore store;
    int node = 1;
    symbols::FieldPlan field;
    field.fieldOffsetBytes = 8;
    field.baseIsPointer = true;
    field.addressTempName = "t0";
    store.setAddressPlan(&node, symbols::AddressPlan { field });

    const auto* plan = store.addressPlan(&node);
    ASSERT_NE(plan, nullptr);
    const auto* f = symbols::get_if<symbols::FieldPlan>(plan);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->fieldOffsetBytes, 8);
    EXPECT_TRUE(f->baseIsPointer);
    EXPECT_EQ(f->addressTempName, "t0");
    EXPECT_EQ(store.addressPlan(&node + 1), nullptr);
}

TEST(AnnotationStore, callPlanRoundTrip) {
    symbols::AnnotationStore store;
    int node = 2;
    symbols::CallPlan plan;
    plan.kind = symbols::CallPlan::Kind::Direct;
    plan.calleeName = "printf";
    store.setCallPlan(&node, plan);

    const auto* got = store.callPlan(&node);
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->kind, symbols::CallPlan::Kind::Direct);
    EXPECT_EQ(got->calleeName, "printf");
    EXPECT_EQ(store.callPlan(&node + 1), nullptr);
}

TEST(AnnotationStore, callPlanIndirect) {
    symbols::AnnotationStore store;
    int node = 6;
    symbols::CallPlan plan;
    plan.kind = symbols::CallPlan::Kind::Indirect;
    plan.calleeName = "fp";
    store.setCallPlan(&node, plan);

    const auto* got = store.callPlan(&node);
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->kind, symbols::CallPlan::Kind::Indirect);
    EXPECT_EQ(got->calleeName, "fp");
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
    store.addStructFieldInit(&node, std::move(a));
    store.addStructFieldInit(&node, std::move(b));

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

TEST(AnnotationStore, clearEmptiesAll) {
    symbols::AnnotationStore store;
    int node = 4;
    symbols::CallPlan plan;
    plan.calleeName = "f";
    store.setCallPlan(&node, plan);
    store.clear();
    EXPECT_EQ(store.callPlan(&node), nullptr);
}

TEST(AnnotationStore, indexPlanVariant) {
    symbols::AnnotationStore store;
    int node = 5;
    symbols::IndexPlan idx;
    idx.elementSize = 4;
    idx.baseIsArray = true;
    idx.addressTempName = "idx";
    store.setAddressPlan(&node, symbols::AddressPlan { idx });
    const auto* plan = store.addressPlan(&node);
    ASSERT_NE(plan, nullptr);
    const auto* i = symbols::get_if<symbols::IndexPlan>(plan);
    ASSERT_NE(i, nullptr);
    EXPECT_EQ(i->elementSize, 4);
    EXPECT_TRUE(i->baseIsArray);
}

} // namespace
