#include "gtest/gtest.h"
#include <set>

#include <memory>
#include <vector>

#include "ast/ConstantExpression.h"
#include "ast/InitializerListExpression.h"
#include "semantic_analyzer/InitializerLowering.h"
#include "semantic_analyzer/SymbolTable.h"
#include "types/ObjectAbi.h"
#include "types/Type.h"

namespace {

using namespace semantic_analyzer;
using namespace ast;

translation_unit::Context ctx() {
    return translation_unit::Context { "test", 1 };
}

std::unique_ptr<Expression> iconst(long v) {
    return std::make_unique<ConstantExpression>(
            Constant { std::to_string(v), type::signedInteger(), ctx() });
}

// Two ints at offsets 0 and 4 pack into one 8-byte word on little-endian layout.
TEST(InitializerLowering, dataWordsPositionalTwoInts) {
    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    EXPECT_EQ(st.getSize(), 8);
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(1));
    elems.emplace_back(iconst(2));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    SymbolTable table;
    type::Type out = st;
    std::vector<std::string> words;
    ASSERT_TRUE(lowerToDataWords(st, list.get(), table, out, words));
    ASSERT_EQ(words.size(), 1u);
    // lane0=1, lane4=2 → 0x00000002_00000001
    EXPECT_EQ(words[0], "0x200000001");
}

TEST(InitializerLowering, dataWordsDesignatedSecondMemberOnly) {
    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    InitializerElement onlyB(iconst(42));
    onlyB.memberPath = { "b" };
    elems.push_back(std::move(onlyB));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    SymbolTable table;
    type::Type out = st;
    std::vector<std::string> words;
    ASSERT_TRUE(lowerToDataWords(st, list.get(), table, out, words));
    ASSERT_EQ(words.size(), 1u);
    // a=0, b=42 → 0x2a << 32
    EXPECT_EQ(words[0], "0x2a00000000");
}

TEST(InitializerLowering, dataWordsArrayPositional) {
    type::Type arr = type::array(type::signedInteger(), 3);
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(10));
    elems.emplace_back(iconst(20));
    elems.emplace_back(iconst(30));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    SymbolTable table;
    type::Type out = arr;
    std::vector<std::string> words;
    ASSERT_TRUE(lowerToDataWords(arr, list.get(), table, out, words));
    EXPECT_EQ(out.getArraySize(), 3);
    EXPECT_EQ(static_cast<int>(words.size()), codegen::object_abi::dataWords(arr.getSize()));
}

TEST(InitializerLowering, fieldInitsZerosUnwrittenMembers) {
    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(7)); // only a; b must be zeroed via onUnwritten path
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    SymbolTable table;
    std::vector<StructFieldInit> fields;
    lowerToFieldInits(st, list.get(), table, [&](StructFieldInit f) {
        fields.push_back(std::move(f));
    });
    std::set<int> offsets;
    bool sawZeroAt4 = false;
    bool sawSevenAt0 = false;
    for (const auto& f : fields) {
        offsets.insert(f.offsetBytes);
        if (f.offsetBytes == 4 && f.zeroInitialize) {
            sawZeroAt4 = true;
        }
        if (f.offsetBytes == 0 && f.constantValue && *f.constantValue == "7") {
            sawSevenAt0 = true;
        }
        if (f.offsetBytes == 0 && !f.zeroInitialize && f.constantValue
                && *f.constantValue == "7") {
            sawSevenAt0 = true;
        }
    }
    EXPECT_TRUE(offsets.count(0)) << "member a at offset 0 missing";
    EXPECT_TRUE(offsets.count(4)) << "unwritten member b at offset 4 must be zeroed";
    EXPECT_TRUE(sawZeroAt4) << "unwritten b must be zeroInitialize";
    EXPECT_TRUE(sawSevenAt0) << "a should fold constant 7";
}

TEST(InitializerLowering, fieldInitsDesignatedNestedPathEmitsStores) {
    type::Type inner = type::structure({
            { "x", type::signedInteger() },
    });
    type::Type outer = type::structure({
            { "inner", inner },
            { "flag", type::signedInteger() },
    });
    const int innerOff = outer.getStructMembers()[0].offsetBytes;
    const int xOff = innerOff + inner.getStructMembers()[0].offsetBytes;
    const int flagOff = outer.getStructMembers()[1].offsetBytes;

    std::vector<InitializerElement> elems;
    InitializerElement des(iconst(9));
    des.memberPath = { "inner", "x" };
    elems.push_back(std::move(des));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    SymbolTable table;
    std::vector<StructFieldInit> fields;
    lowerToFieldInits(outer, list.get(), table, [&](StructFieldInit f) {
        fields.push_back(std::move(f));
    });
    std::set<int> offsets;
    for (const auto& f : fields) {
        offsets.insert(f.offsetBytes);
    }
    // Nested designator writes x; unwritten flag must still be zeroed.
    EXPECT_TRUE(offsets.count(xOff)) << "nested .inner.x offset";
    EXPECT_TRUE(offsets.count(flagOff)) << "unwritten flag must be zeroed";
}

TEST(InitializerLowering, completeArrayFromList) {
    type::Type incomplete = type::array(type::signedInteger(), 0);
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(1));
    elems.emplace_back(iconst(2));
    elems.emplace_back(iconst(3));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    type::Type done = completeArrayTypeFromList(incomplete, list.get());
    EXPECT_TRUE(done.isArray());
    EXPECT_EQ(done.getArraySize(), 3);
}

} // namespace
