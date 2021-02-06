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

    Compiler compiler{ std::make_unique<CompilerComponentsFactory>(
            std::make_unique<ConfigurationParser>(argv.size()-1, argv.data()))};

    ASSERT_THROW(compiler.compile(sourceFile), std::runtime_error);
}

}

