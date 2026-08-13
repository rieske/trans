#include "gtest/gtest.h"

#include "symbols/AddressPlan.h"
#include "symbols/AnnotationTypes.h"
#include "symbols/CallPlan.h"

TEST(AddressBaseMode, leaVsPointer) {
    EXPECT_TRUE(symbols::addressBaseUsesLea(symbols::AddressBaseMode::LeaObject));
    EXPECT_FALSE(symbols::addressBaseUsesLea(symbols::AddressBaseMode::PointerValue));
    EXPECT_TRUE(symbols::addressBaseIsPointerValue(symbols::AddressBaseMode::PointerValue));
}

TEST(AddressPlan, fieldPlanHoldsOffsetAndBitField) {
    symbols::FieldPlan field;
    field.fieldOffsetBytes = 8;
    EXPECT_EQ(field.fieldOffsetBytes, 8);
    EXPECT_FALSE(field.isBitField());
}

TEST(AddressPlan, indexPlanHoldsElementSize) {
    symbols::IndexPlan idx;
    idx.elementSize = 4;
    EXPECT_EQ(idx.elementSize, 4);
}

TEST(AddressPlan, functionDesignatorPlanCarriesName) {
    symbols::FunctionDesignatorPlan d { "foo" };
    EXPECT_EQ(d.functionName, "foo");
}

TEST(AddressPlan, arrayDecayPlanIsDistinctFromResultAddressOf) {
    symbols::AddressPlan decay { symbols::ArrayDecayPlan { "arr" } };
    symbols::AddressPlan addr { symbols::ResultAddressOfPlan {} };
    EXPECT_TRUE(symbols::get_if<symbols::ArrayDecayPlan>(decay));
    EXPECT_FALSE(symbols::get_if<symbols::ResultAddressOfPlan>(decay));
    EXPECT_TRUE(symbols::get_if<symbols::ResultAddressOfPlan>(addr));
    EXPECT_FALSE(symbols::get_if<symbols::ArrayDecayPlan>(addr));
    EXPECT_EQ(symbols::get_if<symbols::ArrayDecayPlan>(decay)->objectName, "arr");
}

TEST(AnnotationTypes, valueAndLabelSlotsExist) {
    using symbols::ValueSlot;
    using symbols::LabelSlot;
    EXPECT_NE(ValueSlot::Result, ValueSlot::Lvalue);
    EXPECT_NE(LabelSlot::Primary, LabelSlot::Exit);
}
