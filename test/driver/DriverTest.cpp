#include "driver/Driver.h"
#include "driver/TranslationUnit.h"
#include "driver/CompilerComponentsFactory.h"
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

class MockCompilerComponentsFactory: public CompilerComponentsFactory {
public:
	std::unique_ptr<Scanner> getScanner() const {
		return std::unique_ptr<Scanner>(getScannerProxy());
	}

	MOCK_CONST_METHOD0(getScannerProxy, Scanner*());
};

class MockScanner: public Scanner {
public:
	MOCK_METHOD1(scan, Token(TranslationUnit& translationUnit));
};

TEST(Driver, invokesCompilerForEachSourceFileName) {
	vector<string> sourceFileNames;

	sourceFileNames.push_back("test/programs/example_prog.src");
	sourceFileNames.push_back("test/programs/test");
	MockConfiguration configuration;
	ON_CALL(configuration, getSourceFileNames()).WillByDefault(ReturnRef(sourceFileNames));

	StrictMock<MockCompiler> compiler;
	EXPECT_CALL(compiler, compile(Property(&TranslationUnit::getFileName, Eq("test/programs/example_prog.src")))).Times(1);
	EXPECT_CALL(compiler, compile(Property(&TranslationUnit::getFileName, Eq("test/programs/test")))).Times(1);

	StrictMock<MockCompilerComponentsFactory> componentsFactory;
	StrictMock<MockScanner> *scanner = new StrictMock<MockScanner>;
	ON_CALL(componentsFactory, getScannerProxy()).WillByDefault(Return(scanner));
	EXPECT_CALL(componentsFactory, getScannerProxy());

	Driver driver(configuration, compiler, componentsFactory);
	driver.run();
}

TEST(Driver, doesNotCompileNonExistentFiles) {
	vector<string> sourceFileNames;

	sourceFileNames.push_back("nonexistent");
	sourceFileNames.push_back("test/programs/example_prog.src");

	MockConfiguration configuration;
	ON_CALL(configuration, getSourceFileNames()).WillByDefault(ReturnRef(sourceFileNames));

	StrictMock<MockCompiler> compiler;
	EXPECT_CALL(compiler, compile(Property(&TranslationUnit::getFileName, Eq("test/programs/example_prog.src")))).Times(1);

	StrictMock<MockCompilerComponentsFactory> componentsFactory;
	StrictMock<MockScanner> *scanner = new StrictMock<MockScanner>;
	ON_CALL(componentsFactory, getScannerProxy()).WillByDefault(Return(scanner));
	EXPECT_CALL(componentsFactory, getScannerProxy());

	Driver driver(configuration, compiler, componentsFactory);
	driver.run();
}
