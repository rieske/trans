#include "gtest/gtest.h"

#include "util/LogManager.h"
#include "util/NullStream.h"

#include <sstream>
#include <stdexcept>

namespace {

TEST(LogManager, restoresWhateverWasInPlaceWhenTheActionThrows) {
    std::ostream* const before = &LogManager::getOutputLogger().stream();
    std::ostringstream out;
    EXPECT_THROW(LogManager::withOutputStreamsForTesting(out, NullStream::getInstance(), []() {
        throw std::logic_error { "internal compiler error" };
    }), std::logic_error);

    EXPECT_EQ(&LogManager::getOutputLogger().stream(), before);
}

TEST(LogManager, theEnclosingCaptureStillCollectsAfterANestedOne) {
    std::ostringstream outer;
    LogManager::withOutputStreamsForTesting(outer, NullStream::getInstance(), []() {
        std::ostringstream inner;
        LogManager::withOutputStreamsForTesting(inner, NullStream::getInstance(), []() {});
        LogManager::getOutputLogger() << "after";
    });
    EXPECT_EQ(outer.str(), "after");
}

} // namespace
