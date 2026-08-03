#include "gtest/gtest.h"
#include "symbols/NodeRef.h"

#include <unordered_map>

#include "symbols/AnnotationStore.h"
#include "symbols/PointerArithPlan.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

translation_unit::Context ctx() { return { "t", 1 }; }

TEST(AnnotationStore, valuesAreNotStoredOnNodeAddressItself) {
    int fakeNode = 0;
    symbols::AnnotationStore store;
    symbols::ValueEntry v { "t0", type::signedInteger(), true, ctx(), 0 };
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
            symbols::ValueEntry { "c", type::signedInteger(), true, ctx(), 0 });
    EXPECT_EQ(store.label(&node, symbols::LabelSlot::LoopEntry)->getName(), "Lentry");
    auto& frame = store.functionFrame(&fn);
    frame.parameterNames = { "a", "b" };
    EXPECT_EQ(store.functionFrameIfAny(&fn)->parameterNames.size(), 2u);
}

TEST(AnnotationStore, structFieldInitsAreSideTable) {
    int node = 3;
    symbols::AnnotationStore store;
    symbols::StructFieldInit init;
    init.offsetBytes = 8;
    init.zeroSpanBytes = 4;
    init.source = std::make_unique<symbols::ValueEntry>("s0", type::signedInteger(), true, ctx(), 0);
    init.address = std::make_unique<symbols::ValueEntry>("a0", type::pointer(type::signedInteger()), true, ctx(), 0);
    store.addStructFieldInit(&node, std::move(init));
    ASSERT_EQ(store.structFieldInits(&node).size(), 1u);
}

TEST(AnnotationStore, typedStringSlots) {
    int node = 4;
    symbols::AnnotationStore store;
    store.setString(&node, symbols::StringSlot::ArrayDecaySource, "arr");
    EXPECT_EQ(*store.string(&node, symbols::StringSlot::ArrayDecaySource), "arr");
}

TEST(AnnotationStore, fieldPlanUsesExpressionRef) {
    int operand = 6;
    int baseNode = 60;
    symbols::AnnotationStore store;
    symbols::FieldPlan field { &baseNode, 16, { symbols::AddressBaseMode::PointerValue, "" } };
    store.setAddressPlan(&operand, symbols::AddressPlan { field });
    const auto* f = symbols::get_if<symbols::FieldPlan>(store.addressPlan(&operand));
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->fieldOffsetBytes, 16);
    EXPECT_EQ(f->base.mode, symbols::AddressBaseMode::PointerValue);
    EXPECT_EQ(f->baseExpr.as<int>(), &baseNode);
}

TEST(AnnotationStore, indexPlanUsesExpressionRef) {
    int arrAccess = 10;
    int base = 1, index = 2;
    symbols::AnnotationStore store;
    symbols::IndexPlan idx { &base, &index, 4, { symbols::AddressBaseMode::LeaObject, "" } };
    store.setAddressPlan(&arrAccess, symbols::AddressPlan { idx });
    const auto* i = symbols::get_if<symbols::IndexPlan>(store.addressPlan(&arrAccess));
    ASSERT_NE(i, nullptr);
    EXPECT_EQ(i->elementSize, 4);
    EXPECT_EQ(i->base.mode, symbols::AddressBaseMode::LeaObject);
    EXPECT_EQ(i->baseExpr.as<int>(), &base);
}

TEST(AnnotationStore, builtinPlanHasOpKind) {
    int call = 11;
    symbols::AnnotationStore store;
    store.setBuiltinPlan(&call, symbols::BuiltinOpPlan { symbols::BuiltinOpKind::Bswap32 });
    ASSERT_NE(store.builtinPlan(&call), nullptr);
    const auto* bop = symbols::get_if<symbols::BuiltinOpPlan>(store.builtinPlan(&call));
    ASSERT_NE(bop, nullptr);
    EXPECT_EQ(bop->opKind, symbols::BuiltinOpKind::Bswap32);
    EXPECT_EQ(store.callPlan(&call), nullptr);
}



TEST(AnnotationStore, nodeRefKeysShareIdentity) {
    int node = 42;
    symbols::AnnotationStore store;
    symbols::NodeRef key { &node };
    store.setResult(key, symbols::ValueEntry { "t", type::signedInteger(), true, ctx(), 0 });
    EXPECT_EQ(store.result(&node)->getName(), "t");
    EXPECT_TRUE(store.hasResult(symbols::NodeRef { &node }));
}

// Miss paths and const overloads (product code rarely probes empty slots).
TEST(AnnotationStore, missPathsAndConstOverloads) {
    int a = 1, b = 2, c = 3;
    symbols::AnnotationStore store;

    EXPECT_EQ(store.value(&a, symbols::ValueSlot::Result), nullptr);
    EXPECT_EQ(store.label(&a, symbols::LabelSlot::LoopEntry), nullptr);
    EXPECT_EQ(store.string(&a, symbols::StringSlot::ArrayDecaySource), nullptr);
    EXPECT_EQ(store.functionFrameIfAny(&a), nullptr);
    EXPECT_EQ(store.functionSymbol(&a), nullptr);
    EXPECT_EQ(store.addressPlan(&a), nullptr);
    EXPECT_EQ(store.pointerArithPlan(&a), nullptr);
    EXPECT_EQ(store.callPlan(&a), nullptr);
    EXPECT_TRUE(store.structFieldInits(&a).empty());

    store.setValue(&b, symbols::ValueSlot::Holder,
            symbols::ValueEntry { "h", type::signedInteger(), true, ctx(), 0 });
    EXPECT_EQ(store.value(&b, symbols::ValueSlot::Result), nullptr);
    EXPECT_FALSE(store.hasValue(&b, symbols::ValueSlot::Result));
    EXPECT_TRUE(store.hasValue(&b, symbols::ValueSlot::Holder));

    store.setLabel(&b, symbols::LabelSlot::LoopExit, symbols::LabelEntry { "Lx" });
    EXPECT_EQ(store.label(&b, symbols::LabelSlot::LoopEntry), nullptr);
    EXPECT_EQ(store.label(&b, symbols::LabelSlot::LoopExit)->getName(), "Lx");

    store.setString(&b, symbols::StringSlot::ConversionTarget, "cvt");
    EXPECT_EQ(store.string(&b, symbols::StringSlot::ArrayDecaySource), nullptr);
    EXPECT_EQ(*store.string(&b, symbols::StringSlot::ConversionTarget), "cvt");

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

    store.setResult(&a, symbols::ValueEntry { "r", type::signedInteger(), true, ctx(), 0 });
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
