#include "TestFixtures.h"

#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"
#include "util/LogManager.h"
#include "util/Process.h"

#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace {

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

TEST(Compiler, assemblyOnlyWritesSFile) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg", "mode_S"};

    std::string outS = program.getSourceFilePath() + ".only.S";
    remove(outS.c_str());
    program.compileWithArgs({
            "--no-preprocess",
            "-S",
            "-o", outS,
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    EXPECT_TRUE(fileExists(outS));
    // Full link must not have run.
    EXPECT_FALSE(fileExists(program.getExecutablePath()));

    std::ifstream in(outS);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_THAT(content, HasSubstr("main"));
}

TEST(Compiler, compileOnlyWritesObjectFile) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 2);
            return 0;
        }
    )prg", "mode_c"};

    std::string outO = program.getSourceFilePath() + ".only.o";
    remove(outO.c_str());
    program.compileWithArgs({
            "--no-preprocess",
            "-c",
            "-o", outO,
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    EXPECT_TRUE(fileExists(outO));
    EXPECT_FALSE(fileExists(program.getExecutablePath()));
}

TEST(Compiler, preprocessOnlyWritesIFile) {
    SourceProgram program{R"prg(
        #define V 3
        int main() {
            printf("%d", V);
            return 0;
        }
    )prg", "mode_E"};

    std::string outI = program.getSourceFilePath() + ".only.i";
    remove(outI.c_str());
    program.compileWithArgs({
            "-E",
            "-o", outI,
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    EXPECT_TRUE(fileExists(outI));
    EXPECT_FALSE(fileExists(program.getExecutablePath()));

    std::ifstream in(outI);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    // Macro should be expanded in the preprocessed output.
    EXPECT_THAT(content, HasSubstr("printf"));
    EXPECT_THAT(content, Not(HasSubstr("#define V")));
}

TEST(Compiler, skipPreprocessFlagAccepted) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 4);
            return 0;
        }
    )prg", "mode_nopre"};

    program.compileWithArgs({
            "--no-preprocess",
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("4");
}

// Parse failure must surface as a non-zero Driver status so make CC=trans stops.
TEST(Compiler, driverReturnsNonZeroOnParseFailure) {
    SourceProgram program{R"prg(
        int main() {
            int x = ({ int y; y = 1; y; });
            return x;
        }
    )prg", "mode_fail_rc"};

    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_NE(status, 0);
    EXPECT_THAT(err.str(), HasSubstr("parsing failed"));
}

// make invokes CC for the final link with only .o files and -l flags.
// trans must forward that to the host linker (non-PIE).
TEST(Compiler, linkOnlyForwardsObjectFiles) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 7);
            return 0;
        }
    )prg", "mode_link_only"};

    std::string obj = program.getSourceFilePath() + ".link.o";
    std::string exe = program.getSourceFilePath() + ".linked";
    remove(obj.c_str());
    remove(exe.c_str());

    program.compileWithArgs({
            "--no-preprocess",
            "-c",
            "-o", obj,
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    ASSERT_TRUE(fileExists(obj));

    std::vector<std::string> arguments {
            "trans", "-r../../../",
            "-o", exe,
            obj
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_EQ(status, 0) << err.str();
    EXPECT_THAT(err.str(), Eq(""));
    ASSERT_TRUE(fileExists(exe));

    // Run linked binary
    std::string outputFile = program.getSourceFilePath() + ".link.out";
    remove(outputFile.c_str());
    ASSERT_EQ(util::runProcess({ exe }, {}, outputFile).exitCode, 0);
    std::ifstream in(outputFile);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_THAT(content, Eq("7"));
}

// -E runs host gcc -E only (no post-preprocess string rewrite layer).
// Attributes may remain in the text; the scanner ignores them. va_list is
// first-class (no inject stage).
TEST(Compiler, preprocessOnlyWritesHostOutput) {
    SourceProgram program{R"prg(
        int main(void) {
            int x __attribute__((unused)) = 1;
            printf("%d", x);
            return 0;
        }
    )prg", "mode_E_preprocess"};

    std::string outI = program.getSourceFilePath() + ".pre.i";
    remove(outI.c_str());

    std::vector<std::string> arguments {
            "trans", "-r../../../",
            "-E", "-o", outI,
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_EQ(status, 0) << err.str();
    ASSERT_TRUE(fileExists(outI));

    std::ifstream in(outI);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_THAT(content, Not(HasSubstr("__trans_va_list")));
    EXPECT_THAT(content, HasSubstr("main"));
}

// make dependency tracking: -MF path must create the depfile (stub is fine).
TEST(Compiler, compileOnlyWritesStubDepFileForMf) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 8);
            return 0;
        }
    )prg", "mode_mf"};

    std::string outO = program.getSourceFilePath() + ".mf.o";
    std::string depDir = program.getSourceFilePath() + ".deps";
    std::string depFile = depDir + "/mode_mf.d";
    remove(outO.c_str());
    remove(depFile.c_str());
    rmdir(depDir.c_str());

    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            "-c", "-MD", "-MF", depFile, "-o", outO,
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_EQ(status, 0) << err.str();
    ASSERT_TRUE(fileExists(outO));
    EXPECT_TRUE(fileExists(depFile)) << "expected stub depfile at " << depFile;
}

// Already-preprocessed .i inputs skip gcc -E. The scanner ignores attributes,
// so a .i with __attribute__ compiles without any string rewrite layer.
TEST(Compiler, preprocessedIFileSkipsHostPreprocess) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg", "mode_i_skip"};

    std::string iFile = program.getSourceFilePath() + ".manual.i";
    {
        std::ofstream out(iFile);
        out << "int main() {\n"
            << "  int x __attribute__((unused)) = 1;\n"
            << "  printf(\"%d\", x);\n"
            << "  return 0;\n"
            << "}\n";
    }
    std::string outO = program.getSourceFilePath() + ".from_i.o";
    remove(outO.c_str());

    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            "-c", "-o", outO, iFile
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_EQ(status, 0) << err.str();
    ASSERT_TRUE(fileExists(outO));
    remove(iFile.c_str());
    remove(outO.c_str());
}

// Nested -MF path: parent directories are created for the stub depfile.
TEST(Compiler, compileOnlyCreatesNestedDepFileDirectory) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 9);
            return 0;
        }
    )prg", "mode_mf_nested"};

    std::string outO = program.getSourceFilePath() + ".nest.o";
    std::string depFile = program.getSourceFilePath() + ".deep/nested/mode.d";
    remove(outO.c_str());
    remove(depFile.c_str());

    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            "-c", "-MF", depFile, "-o", outO,
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_EQ(status, 0) << err.str();
    ASSERT_TRUE(fileExists(outO));
    ASSERT_TRUE(fileExists(depFile));
    // Stub may be empty; existence is what make -include needs.
    std::ifstream in(depFile);
    EXPECT_TRUE(in.good());
}

// Compile-only must fail the driver if the object file is missing or empty
// (matches former scripts/trans-cc empty-object guard).
TEST(Compiler, compileOnlyFailsWhenObjectNotProduced) {
    SourceProgram program{R"prg(
        int main() {
            return 0;
        }
    )prg", "mode_empty_o"};

    // Directory does not exist: nasm cannot write the object.
    std::string outO = program.getSourceFilePath() + ".no_such_dir/out.o";

    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            "-c", "-o", outO,
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_NE(status, 0);
}

// Stub depfile write must fail the driver when the path cannot be created
// (parent path exists as a regular file, so mkdir fails).
TEST(Compiler, compileOnlyFailsWhenDepFileUnwritable) {
    SourceProgram program{R"prg(
        int main() {
            return 0;
        }
    )prg", "mode_mf_fail"};

    std::string outO = program.getSourceFilePath() + ".mf_fail.o";
    std::string blocker = program.getSourceFilePath() + ".mf_blocker";
    std::string depFile = blocker + "/nested.d";
    remove(outO.c_str());
    remove(depFile.c_str());
    {
        std::ofstream block(blocker);
        block << "not a directory";
    }

    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            "-c", "-MF", depFile, "-o", outO,
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_NE(status, 0);
    EXPECT_THAT(err.str(), HasSubstr("depfile"));
    remove(blocker.c_str());
    remove(outO.c_str());
}

// Preprocess injects -D__STDC__=0; host gcc would warn about redefining the
// built-in unless -w is passed. Prove stderr has no redefine warning.
TEST(Compiler, preprocessDoesNotWarnOnStdcRedefine) {
    SourceProgram program{R"prg(
        int main(void) {
            return 0;
        }
    )prg", "mode_stdc_warn"};

    std::string outI = program.getSourceFilePath() + ".stdc.i";
    remove(outI.c_str());

    std::string errPath = program.getSourceFilePath() + ".stdc.err";
    remove(errPath.c_str());
    int errFd = open(errPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(errFd, 0);
    int savedStderr = dup(STDERR_FILENO);
    ASSERT_GE(savedStderr, 0);
    ASSERT_EQ(dup2(errFd, STDERR_FILENO), STDERR_FILENO);
    close(errFd);

    std::vector<std::string> arguments {
            "trans", "-r../../../",
            "-E", "-o", outI,
            program.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    int status = 0;
    LogManager::withOutputStreams(out, err, [&argv, &status]() {
        Driver driver {};
        status = driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });

    fflush(stderr);
    dup2(savedStderr, STDERR_FILENO);
    close(savedStderr);

    EXPECT_EQ(status, 0) << err.str();
    ASSERT_TRUE(fileExists(outI));

    std::ifstream errIn(errPath);
    std::string hostErr((std::istreambuf_iterator<char>(errIn)),
            std::istreambuf_iterator<char>());
    EXPECT_THAT(hostErr, Not(HasSubstr("redefined")));
    EXPECT_THAT(hostErr, Not(HasSubstr("__STDC__")));

    remove(outI.c_str());
    remove(errPath.c_str());
}

// Non-.c sources (functional tests use .src) must still be accepted by host -E
// via -x c on the preprocess command.
TEST(Compiler, preprocessAcceptsNonCExtensionViaXFlag) {
    SourceProgram program{R"prg(
        int main(void) {
            return 0;
        }
    )prg", "mode_x_c"};
    // SourceProgram writes a .src path; -E must succeed (proves -x c).
    std::string outI = program.getSourceFilePath() + ".xc.i";
    remove(outI.c_str());
    program.compileWithArgs({
            "-E",
            "-o", outI,
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    EXPECT_TRUE(fileExists(outI));
    remove(outI.c_str());
}

// Driver compiles each source path independently (each produces its own .out).
TEST(Compiler, multipleSourceFilesEachCompile) {
    SourceProgram a{R"prg(
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg", "multi_a"};
    SourceProgram b{R"prg(
        int main() {
            printf("%d", 2);
            return 0;
        }
    )prg", "multi_b"};

    // Invoke driver once with both sources.
    std::vector<std::string> arguments {
            "trans", "-r../../../", "--no-preprocess",
            a.getSourceFilePath(), b.getSourceFilePath()
    };
    std::vector<char*> argv;
    for (auto& s : arguments) {
        argv.push_back(s.data());
    }
    argv.push_back(nullptr);

    std::stringstream out;
    std::stringstream err;
    LogManager::withOutputStreams(out, err, [&argv]() {
        Driver driver {};
        driver.run(ConfigurationParser {(int)argv.size() - 1, argv.data()});
    });
    EXPECT_THAT(err.str(), Eq(""));
    EXPECT_TRUE(fileExists(a.getExecutablePath()));
    EXPECT_TRUE(fileExists(b.getExecutablePath()));
}

} // namespace
