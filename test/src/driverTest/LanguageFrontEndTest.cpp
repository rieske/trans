#include "driver/Compiler.h"
#include "driver/Configuration.h"
#include "driver/LanguageFrontEnd.h"

#include "gtest/gtest.h"

#include "ResourceHelpers.h"
#include "util/LogManager.h"
#include "util/Logger.h"

#include <iostream>
#include <sstream>

namespace {

Configuration productConfig() {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    return configuration;
}

Configuration dottedProductConfig() {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir() + "./");
    return configuration;
}

class LanguageFrontEndTest: public testing::Test {
protected:
    void SetUp() override {
        LanguageFrontEnd::clearProductCacheForTesting();
    }

    void TearDown() override {
        LanguageFrontEnd::clearProductCacheForTesting();
    }
};

TEST_F(LanguageFrontEndTest, loadReusesInstanceForSamePaths) {
    const auto first = LanguageFrontEnd::load(productConfig());
    const auto second = LanguageFrontEnd::load(productConfig());
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first.get(), second.get());
}

TEST_F(LanguageFrontEndTest, loadDoesNotReuseDifferentResourcePath) {
    const auto first = LanguageFrontEnd::load(productConfig());
    const auto second = LanguageFrontEnd::load(dottedProductConfig());
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first.get(), second.get());

    const auto third = LanguageFrontEnd::load(dottedProductConfig());
    EXPECT_EQ(second.get(), third.get());
}

TEST_F(LanguageFrontEndTest, clearProductCacheDropsTheSlot) {
    const auto first = LanguageFrontEnd::load(productConfig());
    LanguageFrontEnd::clearProductCacheForTesting();
    const auto second = LanguageFrontEnd::load(productConfig());
    EXPECT_NE(first.get(), second.get());
}

TEST_F(LanguageFrontEndTest, compilerConstructionLogsGrammarWhenParserLoggingEnabled) {
    Configuration configuration = productConfig();
    configuration.enableParserLogging();

    std::ostringstream captured;
    auto* previous = std::cout.rdbuf(captured.rdbuf());
    Compiler compiler { configuration };
    std::cout.rdbuf(previous);
    LogManager::registerComponentLogger(Component::PARSER, Logger {});

    const std::string log = captured.str();
    EXPECT_NE(log.find("Terminals:"), std::string::npos) << log;
    EXPECT_NE(log.find("Nonterminals:"), std::string::npos) << log;
}

} // namespace
