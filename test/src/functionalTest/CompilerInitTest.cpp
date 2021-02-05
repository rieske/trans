#include "TestFixtures.h"

#include "driver/CompilerComponentsFactory.h"

namespace {

TEST(Compiler, throwsForNonExistentFile) {
    Compiler compiler{ std::make_unique<CompilerComponentsFactory>(std::make_unique<CompilerConfiguration>())};

    ASSERT_THROW(compiler.compile("nonexistentSourceFileName"), std::runtime_error);
}

}

