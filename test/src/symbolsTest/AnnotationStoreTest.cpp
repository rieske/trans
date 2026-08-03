#include "gtest/gtest.h"
#include "symbols/NodeRef.h"

#include <unordered_map>
#include <variant>

#include "symbols/AnnotationStore.h"
#include "symbols/PointerArithPlan.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

translation_unit::Context ctx() { return { "t", 1 }; }

TEST(AnnotationStore, valuesAreNotStoredOnNodeAddressItself) {
    int fakeNode = 0;
    symbols::AnnotationStore store;
    symbols::ValueEntry v { "t0", type::signedInteger(), ctx(), 0 };
    store.setResult(&fakeNode, v);
    ASSERT_NE(store.value(&fakeNode, symbols::ValueSlot::Result), nullptr);
    EXPECT_EQ(store.result(&fakeNode)->getName(), "t0");
}

TEST(AnnotationStore, labelsAndFunctionFrameShareNodeRecord) {
    int node = 1;
    int fn = 2;
    symbols::AnnotationStore store;
    store.setLabel(&node, symbols::LabelSlot::LoopEntry, symbols::LabelEntry { "Lentry" });
    store.setValue(&node, symbols::ValueSlot::CaseTemp,
            symbols::ValueEntry { "c", type::signedInteger(), ctx(), 0 });
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::LoopEntry)->getName(), "Lentry");
    auto& frame = store.functionFrame(&fn);
    frame.arguments.push_back(symbols::ValueEntry { "a", type::signedInteger(), ctx(), 0 });
    EXPECT_EQ(store.functionFrameIfAny(&fn)->arguments.size(), 1u);
}

TEST(AnnotationStore, callPlanIndirect) {
    symbols::AnnotationStore store;
    int node = 6;
    store.setCallPlan(&node, symbols::CallPlan::Indirect);

    const auto* got = store.callPlan(&node);
    ASSERT_NE(got, nullptr);
    EXPECT_TRUE(symbols::isIndirectCall(*got));
    EXPECT_FALSE(symbols::isDirectCall(*got));
}

TEST(AnnotationStore, fieldInitsAreSideTable) {
    int node = 3;
    symbols::AnnotationStore store;
    symbols::FieldInitTemps temps;
    temps.source = std::make_unique<symbols::ValueEntry>("s0", type::signedInteger(), ctx(), 0);
    temps.address = std::make_unique<symbols::ValueEntry>("a0", type::pointer(type::signedInteger()), ctx(), 0);
    store.addFieldInit(&node, symbols::fieldZeroSpan(8, 4, std::move(temps)));
    ASSERT_EQ(store.fieldInits(&node).size(), 1u);
    EXPECT_TRUE(std::holds_alternative<symbols::FieldZeroSpan>(store.fieldInits(&node)[0]));
}

TEST(AnnotationStore, typedStringSlots) {
    int node = 4;
    symbols::AnnotationStore store;
    store.setString(&node, symbols::StringSlot::ConstantLabel, "arr");
    EXPECT_EQ(*store.string(&node, symbols::StringSlot::ConstantLabel), "arr");
}

TEST(AnnotationStore, fieldPlanRoundTrip) {
    int operand = 6;
    symbols::AnnotationStore store;
    symbols::FieldPlan field;
    field.fieldOffsetBytes = 16;
    store.setAddressPlan(&operand, symbols::AddressPlan { field });
    const auto* f = symbols::get_if<symbols::FieldPlan>(store.addressPlan(&operand));
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->fieldOffsetBytes, 16);
}

TEST(AnnotationStore, indexPlanRoundTrip) {
    int arrAccess = 10;
    symbols::AnnotationStore store;
    symbols::IndexPlan idx;
    idx.elementSize = 4;
    idx.elementType = type::signedInteger();
    idx.baseMode = symbols::AddressBaseMode::LeaObject;
    store.setAddressPlan(&arrAccess, symbols::AddressPlan { idx });
    const auto* plan = store.addressPlan(&arrAccess);
    ASSERT_NE(plan, nullptr);
    const auto* i = symbols::get_if<symbols::IndexPlan>(plan);
    ASSERT_NE(i, nullptr);
    EXPECT_EQ(i->elementSize, 4);
    EXPECT_TRUE(i->elementType.equivalentTo(type::signedInteger()));
    EXPECT_EQ(i->baseMode, symbols::AddressBaseMode::LeaObject);
    EXPECT_TRUE(symbols::addressBaseUsesLea(i->baseMode));
}

TEST(AnnotationStore, builtinPlanHoldsCtzPlan) {
    int call = 11;
    symbols::AnnotationStore store;
    store.setBuiltinPlan(&call, symbols::CtzPlan {});
    ASSERT_NE(store.builtinPlan(&call), nullptr);
    EXPECT_TRUE(symbols::get_if<symbols::CtzPlan>(store.builtinPlan(&call)));
    EXPECT_EQ(store.callPlan(&call), nullptr);
}



TEST(AnnotationStore, nodeRefKeysShareIdentity) {
    int node = 42;
    symbols::AnnotationStore store;
    symbols::NodeRef key { &node };
    store.setResult(key, symbols::ValueEntry { "t", type::signedInteger(), ctx(), 0 });
    EXPECT_EQ(store.result(&node)->getName(), "t");
    EXPECT_TRUE(store.hasResult(symbols::NodeRef { &node }));
}

// Miss paths and const overloads (product code rarely probes empty slots).
TEST(AnnotationStore, missPathsAndConstOverloads) {
    int a = 1, b = 2, c = 3;
    symbols::AnnotationStore store;

    EXPECT_EQ(store.value(&a, symbols::ValueSlot::Result), nullptr);
    EXPECT_EQ(store.label(&a, symbols::LabelSlot::LoopEntry), nullptr);
    EXPECT_EQ(store.string(&a, symbols::StringSlot::ConstantLabel), nullptr);
    EXPECT_EQ(store.functionFrameIfAny(&a), nullptr);
    EXPECT_EQ(store.functionSymbol(&a), nullptr);
    EXPECT_EQ(store.addressPlan(&a), nullptr);
    EXPECT_EQ(store.pointerArithPlan(&a), nullptr);
    EXPECT_EQ(store.callPlan(&a), nullptr);
    EXPECT_TRUE(store.fieldInits(&a).empty());

    store.setValue(&b, symbols::ValueSlot::Holder,
            symbols::ValueEntry { "h", type::signedInteger(), ctx(), 0 });
    EXPECT_EQ(store.value(&b, symbols::ValueSlot::Result), nullptr);
    EXPECT_FALSE(store.hasValue(&b, symbols::ValueSlot::Result));
    EXPECT_TRUE(store.hasValue(&b, symbols::ValueSlot::Holder));

    store.setLabel(&b, symbols::LabelSlot::LoopExit, symbols::LabelEntry { "Lx" });
    EXPECT_EQ(store.label(&b, symbols::LabelSlot::LoopEntry), nullptr);
    EXPECT_EQ(store.label(&b, symbols::LabelSlot::LoopExit)->getName(), "Lx");

    store.setString(&b, symbols::StringSlot::ConstantLabel, "lbl");
    EXPECT_EQ(*store.string(&b, symbols::StringSlot::ConstantLabel), "lbl");
    store.setValue(&b, symbols::ValueSlot::Conversion,
            symbols::ValueEntry { "cvt", type::boolean(), ctx(), 0 });
    EXPECT_EQ(store.value(&b, symbols::ValueSlot::Conversion)->getName(), "cvt");

    type::Type fty = type::function(type::signedInteger(), {});
    store.setFunctionSymbol(&c, symbols::FunctionEntry { "f", fty.getFunction(), ctx() });
    EXPECT_NE(store.functionSymbol(&c), nullptr);
    EXPECT_EQ(store.functionSymbol(&c)->getName(), "f");
    EXPECT_EQ(store.functionSymbol(&a), nullptr);

    const symbols::AnnotationStore& cst = store;
    EXPECT_EQ(cst.value(&a, symbols::ValueSlot::Result), nullptr);
    EXPECT_EQ(cst.label(&a, symbols::LabelSlot::LoopEntry), nullptr);
    EXPECT_EQ(cst.functionSymbol(&a), nullptr);
    EXPECT_EQ(cst.functionFrameIfAny(&a), nullptr);
    EXPECT_NE(cst.value(&b, symbols::ValueSlot::Holder), nullptr);
    EXPECT_EQ(cst.label(&b, symbols::LabelSlot::LoopExit)->getName(), "Lx");
    EXPECT_NE(cst.functionSymbol(&c), nullptr);

    store.setResult(&a, symbols::ValueEntry { "r", type::signedInteger(), ctx(), 0 });
    EXPECT_EQ(store.result(&a)->getName(), "r");
    EXPECT_EQ(cst.result(&a)->getName(), "r");

    store.setPointerArithPlan(&a, symbols::PointerArithPlan {
            symbols::PointerScalePlan { 8, "", true } });
    EXPECT_NE(store.pointerArithPlan(&a), nullptr);
    EXPECT_EQ(store.pointerArithPlan(&b), nullptr);

    store.clear();
    EXPECT_EQ(store.value(&b, symbols::ValueSlot::Holder), nullptr);
}

} // namespace
