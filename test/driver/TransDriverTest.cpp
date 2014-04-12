#include "driver/TransDriver.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <vector>

using namespace testing;
using std::string;
using std::vector;

class MockConfiguration: public Configuration {
public:
	MOCK_CONST_METHOD0(getSourceFileNames, const std::vector<std::string> &());
	MOCK_CONST_METHOD0(getCustomGrammarFileName, const std::string ());
	MOCK_CONST_METHOD0(isParserLoggingEnabled, bool ());
	MOCK_CONST_METHOD0(isScannerLoggingEnabled, bool ());
};

TEST(TransDriver, opensInputFileForReading) {
	MockConfiguration configuration;

	vector<string> sourceFileNames;
	sourceFileNames.push_back("testSource.src");
	ON_CALL(configuration, getSourceFileNames()).WillByDefault(ReturnRef(sourceFileNames));

	TransDriver driver(configuration);
	driver.run();
}
