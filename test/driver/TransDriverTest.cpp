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

class MockCompiler: public Compiler {
public:
	MOCK_METHOD1(compile, void(const std::string fileName));
};

TEST(TransDriver, invokesCompilerForEachSourceFileName) {
	vector<string> sourceFileNames;

	sourceFileNames.push_back("testSource.src");
	sourceFileNames.push_back("testSource2.src");
	MockConfiguration configuration;
	ON_CALL(configuration, getSourceFileNames()).WillByDefault(ReturnRef(sourceFileNames));

	MockCompiler compiler;
	EXPECT_CALL(compiler, compile(StrEq("testSource.src")));
	EXPECT_CALL(compiler, compile(StrEq("testSource2.src")));

	TransDriver driver(configuration, compiler);
	driver.run();
}
