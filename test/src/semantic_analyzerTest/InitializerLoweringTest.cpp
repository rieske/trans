#include "gtest/gtest.h"

#include <memory>
#include <string>
#include <vector>

#include "ast/ConstantExpression.h"
#include "ast/Declarator.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TerminalSymbol.h"
#include "semantic_analyzer/AggregateInitSink.h"
#include "semantic_analyzer/AggregateInitSinks.h"
#include "semantic_analyzer/AggregateInitWalk.h"
#include "semantic_analyzer/CharArrayStringInit.h"
#include "semantic_analyzer/InitializerLowering.h"
#include "semantic_analyzer/SymbolTable.h"
#include "symbols/GlobalInitializer.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

#include <variant>

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

bool isZeroSpan(const symbols::FieldInit& f) {
    return std::holds_alternative<symbols::FieldZeroSpan>(f);
}

bool isZeroSpanAt(const symbols::FieldInit& f, int offset) {
    const auto* z = std::get_if<symbols::FieldZeroSpan>(&f);
    return z && z->offsetBytes == offset;
}

bool isZeroCovering(const symbols::FieldInit& f, int offset, int minSpan) {
    const auto* z = std::get_if<symbols::FieldZeroSpan>(&f);
    return z && z->offsetBytes == offset && z->zeroSpanBytes >= minSpan;
}

bool isConstantAt(const symbols::FieldInit& f, int offset, const char* value) {
    const auto* c = std::get_if<symbols::FieldConstant>(&f);
    return c && c->offsetBytes == offset && c->constantValue == value;
}

TEST(InitializerLowering, completeArrayFromList) {
    type::Type incomplete = type::incompleteArray(type::signedInteger());
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(1));
    elems.emplace_back(iconst(2));
    elems.emplace_back(iconst(3));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    auto bound = incompleteArrayBoundFromInitializer(list.get());
    ASSERT_EQ(bound.kind, IncompleteArrayBound::Kind::Bound);
    EXPECT_EQ(bound.bound, 3);
    type::Type done = type::array(incomplete.getElementType(), bound.bound);
    EXPECT_TRUE(done.isArray());
    EXPECT_EQ(done.getArraySize(), 3);
}

TEST(InitializerLowering, completeArrayFromDesignatedIndex) {
    type::Type incomplete = type::incompleteArray(type::signedInteger());
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(5));
    e.designator.push_back(DesignatorStep::indexWithExpression(iconst(2)));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    auto bound = incompleteArrayBoundFromInitializer(list.get());
    ASSERT_EQ(bound.kind, IncompleteArrayBound::Kind::Bound);
    EXPECT_EQ(bound.bound, 3);
    type::Type done = type::array(incomplete.getElementType(), bound.bound);
    EXPECT_EQ(done.getArraySize(), 3);
}

TEST(InitializerLowering, incompleteArrayBoundEmptyListIsNone) {
    std::vector<InitializerElement> elems;
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    auto bound = incompleteArrayBoundFromInitializer(list.get());
    EXPECT_EQ(bound.kind, IncompleteArrayBound::Kind::None);
}

TEST(InitializerLowering, incompleteArrayBoundNonConstantDesignatorIsError) {
    std::vector<InitializerElement> elems;
    InitializerElement e(iconst(5));
    e.designator.push_back(DesignatorStep::indexWithExpression(
            std::make_unique<IdentifierExpression>("n", ctx())));
    elems.push_back(std::move(e));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    auto bound = incompleteArrayBoundFromInitializer(list.get());
    EXPECT_EQ(bound.kind, IncompleteArrayBound::Kind::Error);
    EXPECT_EQ(bound.error, "designated array index is not a constant expression");
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
    std::vector<symbols::DataWord> words;
    ASSERT_EQ(lowerToDataWords(st, list.get(), host, completed, words), DataWordsLowering::Ok);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    ASSERT_EQ(words.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<symbols::ConstantInit>(words[0]));
    EXPECT_EQ(std::get<symbols::ConstantInit>(words[0]).value, static_cast<long>(0x200000001ull));
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
    std::vector<symbols::DataWord> words;
    ASSERT_EQ(lowerToDataWords(st, list.get(), host, completed, words), DataWordsLowering::Ok);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    ASSERT_EQ(words.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<symbols::ConstantInit>(words[0]));
    EXPECT_EQ(std::get<symbols::ConstantInit>(words[0]).value, static_cast<long>(0x700000000ull));
}

TEST(InitializerLowering, dataWordsUsesCompletedArray) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);

    type::Type incomplete = type::incompleteArray(type::signedInteger());
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(10));
    elems.emplace_back(iconst(20));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    completeIncompleteArrayFromInitializer(incomplete, list.get());
    EXPECT_EQ(incomplete.getArraySize(), 2);

    type::Type completed = type::voidType();
    std::vector<symbols::DataWord> words;
    ASSERT_EQ(lowerToDataWords(incomplete, list.get(), host, completed, words), DataWordsLowering::Ok);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    EXPECT_EQ(completed.getArraySize(), 2);
    ASSERT_EQ(words.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<symbols::ConstantInit>(words[0]));
    EXPECT_EQ(std::get<symbols::ConstantInit>(words[0]).value, static_cast<long>(0x140000000aull));
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

    auto fields = lowerToFieldInits(st, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawLiveAt0 = false;
    // Full-object zero (covers padding) then live store of a=5.
    bool sawFullObjectZero = false;
    for (const auto& f : fields) {
        if (isConstantAt(f, 0, "5")) {
            sawLiveAt0 = true;
        }
        if (isZeroSpanAt(f, 0) || isZeroSpanAt(f, 4)) {
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

    auto fields = lowerToFieldInits(outer, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawLiveAt4 = false;
    bool sawPaddingZero = false;
    for (const auto& f : fields) {
        if (isConstantAt(f, 4, "9")) {
            sawLiveAt4 = true;
        }
        if (isZeroSpan(f)) {
            sawPaddingZero = true;
        }
    }
    EXPECT_TRUE(sawLiveAt4);
    EXPECT_TRUE(sawPaddingZero);
}

// Single value into nested struct fills the first member; remaining members stay zero.
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

    auto fields = lowerToFieldInits(outer, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawLiveAt0 = false;
    bool sawPaddingZero = false;
    for (const auto& f : fields) {
        if (isConstantAt(f, 0, "7")) {
            sawLiveAt0 = true;
        }
        if (isZeroSpan(f)) {
            sawPaddingZero = true;
        }
    }
    EXPECT_TRUE(sawLiveAt0);
    EXPECT_TRUE(sawPaddingZero);
}

// After .a = 1, the next positional fills b; c stays zero.
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

    auto fields = lowerToFieldInits(st, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawA = false;
    bool sawB = false;
    bool sawZeroC = false;
    for (const auto& f : fields) {
        if (isConstantAt(f, 0, "1")) {
            sawA = true;
        }
        if (isConstantAt(f, 4, "2")) {
            sawB = true;
        }
        if (isZeroSpanAt(f, 8) || isZeroCovering(f, 0, 12)) {
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

    auto fields = lowerToFieldInits(outer, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawXY = false;
    bool sawFullObjectZero = false;
    for (const auto& f : fields) {
        if (const auto* s = std::get_if<symbols::FieldStringBytes>(&f)) {
            if (s->offsetBytes == 4 && s->sizeBytes == 8 && s->bytes.size() >= 2
                    && s->bytes[0] == 'x' && s->bytes[1] == 'y') {
                sawXY = true;
            }
        }
        if (isZeroSpanAt(f, 0)) {
            sawFullObjectZero = true;
        }
    }
    EXPECT_TRUE(sawXY);
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

    auto fields = lowerToFieldInits(outer, list.get(), symbols, host);
    ASSERT_FALSE(errors.empty());
    EXPECT_NE(errors.front().find("excess"), std::string::npos) << errors.front();
}

// Empty nested struct is not a slot: { 42 } initializes the next sibling, not excess.
TEST(InitializerLowering, fieldPlanEmptyNestedStructPositionalFallsThrough) {
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
    elems.emplace_back(iconst(42));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));

    auto fields = lowerToFieldInits(outer, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    auto x = type::memberAt(outer, 1);
    ASSERT_TRUE(x.has_value());
    bool sawX = false;
    for (const auto& f : fields) {
        if (isConstantAt(f, x->offsetBytes, "42")) {
            sawX = true;
        }
    }
    EXPECT_TRUE(sawX);
}

TEST(InitializerLowering, classifyNestedBracedStringIsCharArrayString) {
    type::Type chars = type::array(type::signedCharacter(), 8);
    std::vector<InitializerElement> elems;
    elems.emplace_back(std::make_unique<StringLiteralExpression>("\"ab\"", ctx()));
    auto list = std::make_unique<InitializerListExpression>(std::move(elems));
    EXPECT_EQ(classifyObjectInit(chars, list.get()), ObjectInitKind::CharArrayString);

    type::Type bools = type::array(type::boolean(), 4);
    EXPECT_FALSE(isCharArrayStringInit(bools, list.get()));
    EXPECT_EQ(classifyObjectInit(bools, list.get()), ObjectInitKind::AggregateBrace);
}

TEST(InitializerLowering, fieldPlanNestedBracedStringPacksCharArray) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type st = type::structure({
            { "s", type::array(type::signedCharacter(), 8) },
    });
    std::vector<InitializerElement> inner;
    inner.emplace_back(std::make_unique<StringLiteralExpression>("\"ab\"", ctx()));
    std::vector<InitializerElement> outer;
    outer.emplace_back(std::make_unique<InitializerListExpression>(std::move(inner)));
    auto list = std::make_unique<InitializerListExpression>(std::move(outer));

    auto fields = lowerToFieldInits(st, list.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());

    bool sawAB = false;
    for (const auto& f : fields) {
        if (const auto* s = std::get_if<symbols::FieldStringBytes>(&f)) {
            if (s->offsetBytes == 0 && s->sizeBytes == 8 && s->bytes.size() >= 2
                    && s->bytes[0] == 'a' && s->bytes[1] == 'b') {
                sawAB = true;
            }
        }
    }
    EXPECT_TRUE(sawAB);
}

// Zero-size char[] is not a string-pack target; whole-attempt must fail so incomplete path wins.
TEST(InitializerLowering, placeStringArrayRejectsZeroSize) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type zeroChar = type::array(type::signedCharacter(), 0);
    type::Type sizedChar = type::array(type::signedCharacter(), 4);
    auto str = std::make_unique<StringLiteralExpression>("\"ab\"", ctx());
    EXPECT_FALSE(isCharArrayStringInit(zeroChar, str.get()));
    EXPECT_TRUE(isCharArrayStringInit(sizedChar, str.get()));
    std::vector<symbols::FieldInit> plan;
    FieldPlanSink sink { host, symbols, ctx(), plan };
    EXPECT_FALSE(sink.placeStringArray(0, zeroChar, str.get()));
    EXPECT_TRUE(plan.empty());
    EXPECT_TRUE(sink.ok());
}

TEST(InitializerLowering, placeStringArrayRejectsBoolArray) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type boolArr = type::array(type::boolean(), 4);
    type::Type sizedChar = type::array(type::signedCharacter(), 4);
    auto str = std::make_unique<StringLiteralExpression>("\"ab\"", ctx());
    EXPECT_FALSE(isCharArrayStringInit(boolArr, str.get()));
    EXPECT_TRUE(isCharArrayStringInit(sizedChar, str.get()));
    std::vector<symbols::FieldInit> plan;
    FieldPlanSink sink { host, symbols, ctx(), plan };
    EXPECT_FALSE(sink.placeStringArray(0, boolArr, str.get()));
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

    std::vector<symbols::FieldInit> plan;
    FieldPlanSink sink { host, symbols, ctx(), plan };
    EXPECT_TRUE(sink.placeAggregateCopy(0, destType, id.get()));
    EXPECT_FALSE(sink.ok());
    EXPECT_TRUE(plan.empty());
    EXPECT_FALSE(errors.empty());
}

InitializedDeclarator makeNamedDecl(const char* name, std::unique_ptr<Expression> init) {
    return InitializedDeclarator(
            std::make_unique<Declarator>(
                    std::make_unique<Identifier>(TerminalSymbol { "id", name, ctx() })),
            std::move(init));
}

// Incomplete char[] + string completes size; packing is sink placeStringArray only.
TEST(InitializerLowering, completeCharArrayFromStringThenPack) {
    symbols::AnnotationStore ann;
    std::vector<std::string> errors;
    AggregateInitHost host = makeHost(ann, errors);
    SymbolTable symbols;

    type::Type type = type::incompleteArray(type::signedCharacter());
    auto str = std::make_unique<StringLiteralExpression>("\"ab\"", ctx());
    completeIncompleteArrayFromInitializer(type, str.get());
    EXPECT_FALSE(type.isIncompleteArray());
    EXPECT_EQ(type.getArraySize(), 3);

    auto fields = lowerToFieldInits(type, str.get(), symbols, host);
    ASSERT_TRUE(errors.empty()) << (errors.empty() ? "" : errors.front());
    ASSERT_EQ(fields.size(), 1u);
    const auto* packed = std::get_if<symbols::FieldStringBytes>(&fields[0]);
    ASSERT_NE(packed, nullptr);
    EXPECT_EQ(packed->offsetBytes, 0);
    EXPECT_EQ(packed->sizeBytes, 3);
    ASSERT_GE(packed->bytes.size(), 2u);
    EXPECT_EQ(packed->bytes[0], 'a');
    EXPECT_EQ(packed->bytes[1], 'b');
}

TEST(InitializerLowering, completeDoesNotResizeSizedZeroArray) {
    type::Type type = type::array(type::signedInteger(), 0);
    std::vector<InitializerElement> elems;
    elems.emplace_back(iconst(1));
    auto decl = makeNamedDecl("z", std::make_unique<InitializerListExpression>(std::move(elems)));
    completeIncompleteArrayFromInitializer(type, decl.getInitializer());
    EXPECT_FALSE(type.isIncompleteArray());
    EXPECT_EQ(type.getArraySize(), 0);
}

} // namespace
