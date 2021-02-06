#include "TestFixtures.h"

#include "driver/CompilerComponentsFactory.h"
#include "driver/ConfigurationParser.h"

namespace {

TEST(Compiler, throwsForNonExistentFile) {
    std::string sourceFile = "nonexistentSourceFileName";
    std::vector<std::string> arguments {"trans", "-r../../../", sourceFile};
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    ConfigurationParser configurationParser { (int)argv.size()-1, argv.data() };
    Compiler compiler{ CompilerComponentsFactory { configurationParser.parseConfiguration() } };

    ASSERT_THROW(compiler.compile(sourceFile), std::runtime_error);
}

}

