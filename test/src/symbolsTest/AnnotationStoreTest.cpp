#include "gtest/gtest.h"

#include "symbols/AnnotationStore.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

translation_unit::Context ctx() { return { "t", 1 }; }

TEST(AnnotationStore, valuesAreNotStoredOnNodeAddressItself) {
    int fakeNode = 0;
    symbols::AnnotationStore store;
    symbols::AnnotationStore::Bind bind(store);

    symbols::ValueEntry v { "t0", type::signedInteger(), true, ctx(), 0 };
    store.setValue(&fakeNode, symbols::ValueSlot::Result, v);

    ASSERT_NE(store.value(&fakeNode, symbols::ValueSlot::Result), nullptr);
    EXPECT_EQ(store.value(&fakeNode, symbols::ValueSlot::Result)->getName(), "t0");
    EXPECT_EQ(store.value(&fakeNode, symbols::ValueSlot::Lvalue), nullptr);
}

TEST(AnnotationStore, currentRequiresBind) {
    EXPECT_FALSE(symbols::AnnotationStore::hasCurrent());
    symbols::AnnotationStore store;
    {
        symbols::AnnotationStore::Bind bind(store);
        EXPECT_TRUE(symbols::AnnotationStore::hasCurrent());
        EXPECT_EQ(&symbols::AnnotationStore::current(), &store);
    }
    EXPECT_FALSE(symbols::AnnotationStore::hasCurrent());
}

TEST(AnnotationStore, labelsAndFunctionFrame) {
    int node = 1;
    int fn = 2;
    symbols::AnnotationStore store;
    symbols::AnnotationStore::Bind bind(store);

    store.setLabel(&node, symbols::LabelSlot::LoopEntry, symbols::LabelEntry { "Lentry" });
    ASSERT_NE(store.label(&node, symbols::LabelSlot::LoopEntry), nullptr);
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::LoopEntry)->getName(), "Lentry");

    auto& frame = store.functionFrame(&fn);
    frame.parameterNames = { "a", "b" };
    EXPECT_EQ(store.functionFrameIfAny(&fn)->parameterNames.size(), 2u);
}

} // namespace
