#include "gtest/gtest.h"

#include "symbols/AddressPlan.h"
#include "symbols/AnnotationTypes.h"
#include "symbols/CallPlan.h"

TEST(AddressBaseMode, leaVsPointer) {
    EXPECT_TRUE(symbols::addressBaseUsesLea(symbols::AddressBaseMode::LeaObject));
    EXPECT_FALSE(symbols::addressBaseUsesLea(symbols::AddressBaseMode::PointerValue));
    EXPECT_TRUE(symbols::addressBaseIsPointerValue(symbols::AddressBaseMode::PointerValue));
}

TEST(AddressBaseResolved, packagesModeAndName) {
    symbols::AddressBaseResolved r { symbols::AddressBaseMode::PointerValue, "t0" };
    EXPECT_EQ(r.mode, symbols::AddressBaseMode::PointerValue);
    EXPECT_EQ(r.name, "t0");
}

TEST(AddressPlan, fieldPlanEmbedsAddressBaseResolved) {
    int baseNode = 1;
    symbols::FieldPlan field;
    field.baseExpr = &baseNode;
    field.fieldOffsetBytes = 8;
    field.base = { symbols::AddressBaseMode::PointerValue, "t_base" };
    EXPECT_EQ(field.base.name, "t_base");
    EXPECT_EQ(field.base.mode, symbols::AddressBaseMode::PointerValue);
}

TEST(AddressPlan, indexPlanEmbedsAddressBaseResolved) {
    int b = 0, i = 1;
    symbols::IndexPlan idx;
    idx.baseExpr = &b;
    idx.indexExpr = &i;
    idx.elementSize = 4;
    idx.base = { symbols::AddressBaseMode::LeaObject, "arr" };
    EXPECT_TRUE(symbols::addressBaseUsesLea(idx.base.mode));
    EXPECT_EQ(idx.base.name, "arr");
}

TEST(AddressPlan, functionDesignatorPlanCarriesName) {
    symbols::FunctionDesignatorPlan d { "foo" };
    EXPECT_EQ(d.functionName, "foo");
}

TEST(AnnotationTypes, valueAndLabelSlotsExist) {
    using symbols::ValueSlot;
    using symbols::LabelSlot;
    EXPECT_NE(ValueSlot::Result, ValueSlot::Lvalue);
    EXPECT_NE(LabelSlot::Primary, LabelSlot::Exit);
}
