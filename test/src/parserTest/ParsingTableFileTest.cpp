#include "parser/ParsingTableFile.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <sstream>
#include <string>

using namespace parser;
using testing::ElementsAre;
using testing::Eq;

TEST(ParsingTableFile, roundTripsRecords) {
    std::stringstream stream;
    ParsingTableWriter writer { stream };
    writer.writeHeader(3);
    writer.writeActions({
            { 0, 14, "s 5" },
            { 1, 22, "r 2" },
            { 2, 0, "a" },
    });
    writer.writeErrors({ { 2, { 14, 22 } } });
    writer.writeGotos({ { 0, 3, 1 } });

    stream.seekg(0);
    ParsingTableReader reader { stream };
    EXPECT_THAT(reader.readHeader(), Eq(3u));

    const auto actions = reader.readActions();
    ASSERT_THAT(actions.size(), Eq(3u));
    EXPECT_THAT(actions[0].state, Eq(0u));
    EXPECT_THAT(actions[0].terminal, Eq(14));
    EXPECT_THAT(actions[0].serialized, Eq("s 5"));
    EXPECT_THAT(actions[2].serialized, Eq("a"));

    const auto errors = reader.readErrors();
    ASSERT_THAT(errors.size(), Eq(1u));
    EXPECT_THAT(errors[0].state, Eq(2u));
    EXPECT_THAT(errors[0].candidates, ElementsAre(14, 22));

    const auto gotos = reader.readGotos();
    ASSERT_THAT(gotos.size(), Eq(1u));
    EXPECT_THAT(gotos[0].fromState, Eq(0u));
    EXPECT_THAT(gotos[0].nonterminal, Eq(3));
    EXPECT_THAT(gotos[0].toState, Eq(1u));
}

TEST(ParsingTableFile, rejectsUnknownKind) {
    std::stringstream stream { "12\n%%\n" };
    ParsingTableReader reader { stream };
    EXPECT_THROW(reader.readHeader(), std::runtime_error);
}

TEST(ParsingTableFile, rejectsUnknownVersion) {
    std::stringstream stream { "sparse 2\n3\n%%\n" };
    ParsingTableReader reader { stream };
    EXPECT_THROW(reader.readHeader(), std::runtime_error);
}
