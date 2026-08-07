#include "gtest/gtest.h"

#include <memory>
#include <string>
#include <vector>

#include "ast/ConstantExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "semantic_analyzer/AggregateInitSink.h"
#include "semantic_analyzer/AggregateInitSinks.h"
#include "semantic_analyzer/AggregateInitWalk.h"
#include "semantic_analyzer/InitializerLowering.h"
#include "semantic_analyzer/SymbolTable.h"
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

AggregateInitHost makeHost(symbols::AnnotationStore& ann, std::vector<std::string>& errors) {
    return AggregateInitHost {
        ann,
        [&](std::string msg, const translation_unit::Context&) {
            errors.push_back(std::move(msg));
        }
    };
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

TEST(InitializerLowering, completeArrayFromDesignatedIndex) {
    type::Type incomplete = type::array(type::signedInteger(), 0);
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(5));
    e.designator.push_back(DesignatorStep::indexWithExpression(iconst(2)));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    type::Type done = completeArrayTypeFromList(incomplete, list.get());
    EXPECT_EQ(done.getArraySize(), 3);
}

TEST(InitializerLowering, dataWordsPacksTwoInts) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);

    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(1));
    elems.emplace_back(iconst(2));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    type::Type completed = type::voidType();
    std::vector<std::string> words;
    ASSERT_TRUE(lowerToDataWords(st, list.get(), host, completed, words));
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    ASSERT_EQ(words.size(), 1u);
    EXPECT_EQ(words[0], "0x200000001");
}

TEST(InitializerLowering, dataWordsDesignatedSecondMember) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);

    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(7));
    e.designator.push_back(DesignatorStep::member("b"));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    type::Type completed = type::voidType();
    std::vector<std::string> words;
    ASSERT_TRUE(lowerToDataWords(st, list.get(), host, completed, words));
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    ASSERT_EQ(words.size(), 1u);
    EXPECT_EQ(words[0], "0x700000000");
}

TEST(InitializerLowering, dataWordsCompletesIncompleteArray) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);

    type::Type incomplete = type::array(type::signedInteger(), 0);
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(10));
    elems.emplace_back(iconst(20));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    type::Type completed = type::voidType();
    std::vector<std::string> words;
    ASSERT_TRUE(lowerToDataWords(incomplete, list.get(), host, completed, words));
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    EXPECT_EQ(completed.getArraySize(), 2);
    ASSERT_EQ(words.size(), 1u);
    EXPECT_EQ(words[0], "0x140000000a");
}

TEST(InitializerLowering, fieldPlanResidualZeroOffsets) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(5));
    e.designator.push_back(DesignatorStep::member("a"));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    std::vector<symbols::StructFieldInit> fields;
    lowerToFieldInits(st, list.get(), symbols, host,
            [&](symbols::StructFieldInit f) { fields.push_back(std::move(f)); });
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawLiveAt0 = false;
    // Full-object zero (covers padding) then live store of a=5.
    bool sawFullObjectZero = false;
    for (const auto& f : fields) {
        if (f.offsetBytes == 0 && f.zeroSpanBytes <= 0 && f.constantValue
                && *f.constantValue == "5") {
            sawLiveAt0 = true;
        }
        if (f.zeroSpanBytes > 0 && f.offsetBytes == 0) {
            sawFullObjectZero = true;
        }
        // Member-wise residual zero at b (offset 4) also acceptable.
        if (f.offsetBytes == 4 && f.zeroSpanBytes > 0) {
            sawFullObjectZero = true;
        }
    }
    EXPECT_TRUE(sawLiveAt0);
    EXPECT_TRUE(sawFullObjectZero);
}

TEST(InitializerLowering, fieldPlanNestedDesignator) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type inner = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    type::Type outer = type::structure({
            { "in", inner },
            { "c", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(9));
    e.designator.push_back(DesignatorStep::member("in"));
    e.designator.push_back(DesignatorStep::member("b"));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    std::vector<symbols::StructFieldInit> fields;
    lowerToFieldInits(outer, list.get(), symbols, host,
            [&](symbols::StructFieldInit f) { fields.push_back(std::move(f)); });
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawLiveAt4 = false;
    bool sawPaddingZero = false;
    for (const auto& f : fields) {
        if (f.offsetBytes == 4 && f.zeroSpanBytes <= 0 && f.constantValue
                && *f.constantValue == "9") {
            sawLiveAt4 = true;
        }
        // Full-object or residual span zero covers offsets 0 and/or 8.
        if (f.zeroSpanBytes > 0) {
            sawPaddingZero = true;
        }
        if (f.zeroSpanBytes > 0 && (f.offsetBytes == 0 || f.offsetBytes == 8)) {
            sawPaddingZero = true;
        }
    }
    EXPECT_TRUE(sawLiveAt4);
    EXPECT_TRUE(sawPaddingZero);
}

// Stream brace elision into nested struct: single value fills first member, siblings zero.
// Pins placeSlotsFrom ZeroThrough (fill nested slots via placeIntoSlot barrier).
TEST(InitializerLowering, fieldPlanStreamNestedElisionZerosSiblings) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type inner = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    type::Type outer = type::structure({
            { "in", inner },
            { "x", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(7));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    std::vector<symbols::StructFieldInit> fields;
    lowerToFieldInits(outer, list.get(), symbols, host,
            [&](symbols::StructFieldInit f) { fields.push_back(std::move(f)); });
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawLiveAt0 = false;
    bool sawPaddingZero = false;
    for (const auto& f : fields) {
        if (f.offsetBytes == 0 && f.zeroSpanBytes <= 0 && f.constantValue
                && *f.constantValue == "7") {
            sawLiveAt0 = true;
        }
        if (f.zeroSpanBytes > 0) {
            sawPaddingZero = true;
        }
    }
    EXPECT_TRUE(sawLiveAt0);
    EXPECT_TRUE(sawPaddingZero);
}

// After designator, positional resume fills next sibling then outer residual zeros the rest.
// Pins placeSlotsFrom StopEarly (next designator is not zero-through-consumed as nested fill).
TEST(InitializerLowering, fieldPlanDesignatorResumePositional) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
            { "c", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    {
        InitializerElement e(iconst(1));
        e.designator.push_back(DesignatorStep::member("a"));
        elems.push_back(std::move(e));
    }
    elems.emplace_back(iconst(2));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    std::vector<symbols::StructFieldInit> fields;
    lowerToFieldInits(st, list.get(), symbols, host,
            [&](symbols::StructFieldInit f) { fields.push_back(std::move(f)); });
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawA = false;
    bool sawB = false;
    bool sawZeroC = false;
    for (const auto& f : fields) {
        if (f.offsetBytes == 0 && f.zeroSpanBytes <= 0 && f.constantValue
                && *f.constantValue == "1") {
            sawA = true;
        }
        if (f.offsetBytes == 4 && f.zeroSpanBytes <= 0 && f.constantValue
                && *f.constantValue == "2") {
            sawB = true;
        }
        // Span zero covering c, or residual zero at offset 8.
        if (f.zeroSpanBytes > 0
                && (f.offsetBytes == 8
                        || (f.offsetBytes == 0 && f.zeroSpanBytes >= 12))) {
            sawZeroC = true;
        }
    }
    EXPECT_TRUE(sawA);
    EXPECT_TRUE(sawB);
    EXPECT_TRUE(sawZeroC);
}

// Designator .in = "xy" where Inner starts with char[8]: pack string into nested array.
// Full-object zero covers padding; string bytes overwrite the char array start.
TEST(InitializerLowering, fieldPlanDesignatedNestedStringElision) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type inner = type::structure({
            { "c", type::array(type::signedCharacter(), 8) },
            { "flag", type::signedInteger() },
    });
    type::Type outer = type::structure({
            { "tag", type::signedInteger() },
            { "in", inner },
    });

    // Build .in = "xy" without a full SA pass: use string literal expression.
    auto str = std::make_unique<StringLiteralExpression>("\"xy\"", ctx());
    // StringLiteral needs constant symbol for some paths; field plan packs via decode.
    std::vector<InitializerElement> elems;
    InitializerElement e(std::move(str));
    e.designator.push_back(DesignatorStep::member("in"));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    std::vector<symbols::StructFieldInit> fields;
    lowerToFieldInits(outer, list.get(), symbols, host,
            [&](symbols::StructFieldInit f) { fields.push_back(std::move(f)); });
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    // 'x' at offset of in.c[0] = 4; 'y' at 5; whole outer zeroed first (padding-safe).
    bool sawX = false;
    bool sawY = false;
    bool sawFullObjectZero = false;
    for (const auto& f : fields) {
        if (f.offsetBytes == 4 && f.constantValue && *f.constantValue == "120") {
            sawX = true;
        }
        if (f.offsetBytes == 5 && f.constantValue && *f.constantValue == "121") {
            sawY = true;
        }
        if (f.zeroSpanBytes > 0 && f.offsetBytes == 0) {
            sawFullObjectZero = true;
        }
    }
    EXPECT_TRUE(sawX);
    EXPECT_TRUE(sawY);
    EXPECT_TRUE(sawFullObjectZero);
}

// Peel into empty nested structure with a live value must emit excess, not drop silently.
TEST(InitializerLowering, fieldPlanEmptyNestedStructLiveValueExcess) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type empty = type::structure({});
    type::Type outer = type::structure({
            { "e", empty },
            { "x", type::signedInteger() },
    });
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(5));
    e.designator.push_back(DesignatorStep::member("e"));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    std::vector<symbols::StructFieldInit> fields;
    lowerToFieldInits(outer, list.get(), symbols, host,
            [&](symbols::StructFieldInit f) { fields.push_back(std::move(f)); });
    ASSERT_FALSE(errors.empty());
    EXPECT_NE(errors.front().find("excess"), std::string::npos) << errors.front();
}

// Zero-size char[] is not a string-pack target; whole-attempt must fail so incomplete path wins.
TEST(InitializerLowering, placeStringArrayRejectsZeroSize) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type zeroChar = type::array(type::signedCharacter(), 0);
    EXPECT_FALSE(isCharArrayType(zeroChar));
    EXPECT_TRUE(isCharArrayType(type::array(type::signedCharacter(), 4)));

    auto str = std::make_unique<StringLiteralExpression>("\"ab\"", ctx());
    std::vector<symbols::StructFieldInit> plan;
    FieldPlanSink sink { host, symbols, ctx(), plan };
    EXPECT_FALSE(sink.placeStringArray(0, zeroChar, str.get()));
    EXPECT_TRUE(plan.empty());
    EXPECT_TRUE(sink.ok());
}

// Incompatible record-record copy fails the sink and leaves an empty plan.
TEST(InitializerLowering, fieldPlanAssignFailureDropsPlan) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type srcType = type::structure({ { "x", type::signedInteger() } });
    type::Type destType = type::structure({ { "y", type::signedLong() } });

    auto id = std::make_unique<IdentifierExpression>("src", ctx());
    auto srcSym = symbols.createTemporarySymbol(srcType);
    id->setTypeAndResult(ann, srcSym);

    std::vector<symbols::StructFieldInit> plan;
    FieldPlanSink sink { host, symbols, ctx(), plan };
    EXPECT_TRUE(sink.placeAggregateCopy(0, destType, id.get()));
    EXPECT_FALSE(sink.ok());
    EXPECT_TRUE(plan.empty());
    EXPECT_FALSE(errors.empty());
}

} // namespace
