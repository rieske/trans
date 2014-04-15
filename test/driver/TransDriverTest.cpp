#include "driver/TransDriver.h"
#include "driver/TranslationUnit.h"
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
	MOCK_CONST_METHOD1(compile, void(TranslationUnit& translationUnit));
};

TEST(TransDriver, invokesCompilerForEachSourceFileName) {
	vector<string> sourceFileNames;

	sourceFileNames.push_back("testSource.src");
	sourceFileNames.push_back("testSource2.src");
	MockConfiguration configuration;
	ON_CALL(configuration, getSourceFileNames()).WillByDefault(ReturnRef(sourceFileNames));

	StrictMock<MockCompiler> compiler;
	EXPECT_CALL(compiler, compile(A<TranslationUnit&>())).Times(2);

	TransDriver driver(configuration, compiler);
	driver.run();
}
