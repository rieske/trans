#include "gtest/gtest.h"

#include "codegen/FrameSymbol.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace {

using codegen::frameSymbolFrom;
using codegen::FrameSymbol;

TEST(FrameSymbol, mapsIntegralValueEntry) {
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry e { "x", type::signedInteger(), false, ctx, 3 };
    FrameSymbol fs = frameSymbolFrom(e, false);
    EXPECT_EQ(fs.name, "x");
    EXPECT_EQ(fs.index, 3);
    EXPECT_EQ(fs.sizeBytes(), 4);
    EXPECT_TRUE(fs.isSigned());
    EXPECT_FALSE(fs.isFloating());
    EXPECT_EQ(fs.valueKind(), codegen::ValueKind::INTEGRAL);

    codegen::Value v = fs.toValue();
    EXPECT_EQ(v.getName(), "x");
    EXPECT_EQ(v.getIndex(), 3);
    EXPECT_EQ(v.getSizeInBytes(), 4);
    EXPECT_TRUE(v.isSigned());
    EXPECT_EQ(v.getValueKind(), codegen::ValueKind::INTEGRAL);
}

TEST(FrameSymbol, mapsUnsignedAndFloating) {
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry u { "u", type::unsignedInteger(), true, ctx, 0 };
    FrameSymbol fu = frameSymbolFrom(u, true);
    EXPECT_FALSE(fu.isSigned());
    EXPECT_TRUE(fu.isTemp);
    EXPECT_EQ(fu.sizeBytes(), 4);

    symbols::ValueEntry d { "d", type::doubleFloating(), true, ctx, 1 };
    FrameSymbol fd = frameSymbolFrom(d, true);
    EXPECT_TRUE(fd.isFloating());
    EXPECT_EQ(fd.valueKind(), codegen::ValueKind::FLOATING);
    EXPECT_EQ(fd.sizeBytes(), 8);
    EXPECT_TRUE(fd.isSigned()); // non-integral default signed for stack policy
}

TEST(FrameSymbol, mapsPointerAndAggregateSizes) {
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry p { "p", type::pointer(type::signedInteger()), false, ctx, 0 };
    EXPECT_EQ(frameSymbolFrom(p).sizeBytes(), 8);
    EXPECT_EQ(frameSymbolFrom(p).valueKind(), codegen::ValueKind::INTEGRAL);

    auto st = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    symbols::ValueEntry s { "s", st, false, ctx, 0 };
    EXPECT_EQ(frameSymbolFrom(s).sizeBytes(), st.getSize());
    EXPECT_GT(frameSymbolFrom(s).sizeBytes(), 16);
}

TEST(FrameSymbol, emptyTypeGetsWordHome) {
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry e { "v", type::voidType(), true, ctx, 0 };
    EXPECT_EQ(frameSymbolFrom(e).sizeBytes(), 8); // valueSizeBytes empty → word
}

// Frame packing assigns stack slots independently of ValueEntry index.
TEST(FrameSymbol, toValueAtSlotOverridesEntryIndex) {
    translation_unit::Context ctx { "t", 1 };
    symbols::ValueEntry e { "tmp", type::signedLong(), true, ctx, 99 };
    FrameSymbol fs = frameSymbolFrom(e, true);
    codegen::Value v = fs.toValueAtSlot(7);
    EXPECT_EQ(v.getName(), "tmp");
    EXPECT_EQ(v.getIndex(), 7);
    EXPECT_EQ(v.getSizeInBytes(), 8);
    EXPECT_EQ(fs.toValue().getIndex(), 99); // entry index unchanged for args
}

} // namespace
