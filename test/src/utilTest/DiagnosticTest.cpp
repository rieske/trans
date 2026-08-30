#include "gtest/gtest.h"

#include "util/Diagnostic.h"

#include <sstream>

TEST(Sink, emptyHasNoErrors) {
    std::ostringstream logged;
    diag::Sink sink(logged);
    EXPECT_FALSE(sink.hasErrors());
    EXPECT_TRUE(sink.all().empty());
    std::ostringstream formatted;
    sink.formatTo(formatted);
    EXPECT_EQ(formatted.str(), "");
    EXPECT_EQ(logged.str(), "");
}

TEST(Sink, errorRecordsAndWritesToInjectedStream) {
    std::ostringstream logged;
    diag::Sink sink(logged);
    sink.error({ "t.c", 3 }, "variable `a` declared void");

    EXPECT_TRUE(sink.hasErrors());
    ASSERT_EQ(sink.all().size(), 1u);
    EXPECT_EQ(sink.all()[0].severity, diag::Severity::Error);
    EXPECT_EQ(sink.all()[0].where.getSourceName(), "t.c");
    EXPECT_EQ(sink.all()[0].where.getOffset(), 3u);
    EXPECT_EQ(sink.all()[0].message, "variable `a` declared void");

    std::ostringstream formatted;
    sink.formatTo(formatted);
    EXPECT_EQ(formatted.str(), "t.c:3: error: variable `a` declared void\n");
    EXPECT_EQ(logged.str(), formatted.str());
}

TEST(Sink, warnDoesNotCountAsError) {
    std::ostringstream logged;
    diag::Sink sink(logged);
    sink.warn({ "t.c", 1 }, "unused");
    EXPECT_FALSE(sink.hasErrors());
    ASSERT_EQ(sink.all().size(), 1u);
    EXPECT_EQ(sink.all()[0].severity, diag::Severity::Warning);

    std::ostringstream formatted;
    sink.formatTo(formatted);
    EXPECT_EQ(formatted.str(), "t.c:1: warning: unused\n");
    EXPECT_EQ(logged.str(), formatted.str());
}

TEST(Sink, formatToEmitsAllInOrder) {
    std::ostringstream logged;
    diag::Sink sink(logged);
    sink.error({ "a.c", 2 }, "first");
    sink.warn({ "a.c", 4 }, "second");
    sink.error({ "a.c", 5 }, "third");
    EXPECT_TRUE(sink.hasErrors());
    EXPECT_EQ(sink.all().size(), 3u);

    std::ostringstream formatted;
    sink.formatTo(formatted);
    EXPECT_EQ(formatted.str(),
            "a.c:2: error: first\n"
            "a.c:4: warning: second\n"
            "a.c:5: error: third\n");
    EXPECT_EQ(logged.str(), formatted.str());
}
