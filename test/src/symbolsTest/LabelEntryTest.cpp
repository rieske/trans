#include "gtest/gtest.h"

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace {

TEST(LabelEntry, name) {
    symbols::LabelEntry lab { "L1" };
    EXPECT_EQ(lab.getName(), "L1");
}

TEST(AnnotationStore, labelSlots) {
    symbols::AnnotationStore store;
    int node = 0;
    store.setLabel(&node, symbols::LabelSlot::Falsy, symbols::LabelEntry { "Lf" });
    store.setLabel(&node, symbols::LabelSlot::Exit, symbols::LabelEntry { "Le" });
    ASSERT_TRUE(store.hasLabel(&node, symbols::LabelSlot::Falsy));
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Falsy)->getName(), "Lf");
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::Exit)->getName(), "Le");
    EXPECT_FALSE(store.hasLabel(&node, symbols::LabelSlot::Truthy));
}

} // namespace
