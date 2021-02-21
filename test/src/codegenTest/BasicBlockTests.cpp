#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>
#include <stdexcept>

#include "codegen/CodeGeneratingVisitor.h"
#include "codegen/quadruples/BasicBlock.h"
#include "codegen/quadruples/Inc.h"
#include "codegen/quadruples/Dec.h"
#include "codegen/quadruples/Jump.h"
#include "codegen/quadruples/Label.h"
#include "codegen/quadruples/VoidReturn.h"

namespace {

using namespace testing;
using namespace codegen;

void assertBasicBlockEquals(BasicBlock& bb, std::string expected) {
    std::stringstream bbOutput;
    bbOutput << bb;
    EXPECT_THAT(bbOutput.str(), StrEq(expected));
}

TEST(BasicBlock, identifiesBasicBlocks) {
    std::vector<std::unique_ptr<Quadruple>> instructions{};
    instructions.push_back(std::make_unique<Inc>("foo"));
    instructions.push_back(std::make_unique<Label>("label"));
    instructions.push_back(std::make_unique<Inc>("foo"));
    instructions.push_back(std::make_unique<Dec>("foo"));
    instructions.push_back(std::make_unique<Jump>("label"));
    instructions.push_back(std::make_unique<Dec>("foo"));
    instructions.push_back(std::make_unique<Dec>("foo"));
    instructions.push_back(std::make_unique<Label>("label"));
    instructions.push_back(std::make_unique<Dec>("foo"));
    instructions.push_back(std::make_unique<Dec>("foo"));
    instructions.push_back(std::make_unique<Jump>("label", JumpCondition::IF_ABOVE));
    instructions.push_back(std::make_unique<Inc>("foo"));
    instructions.push_back(std::make_unique<Inc>("foo"));
    instructions.push_back(std::make_unique<Inc>("foo"));

    auto basicBlocks = toBasicBlocks(std::move(instructions));

    EXPECT_THAT(basicBlocks, SizeIs(5));
    for (const auto& inst : instructions) {
        EXPECT_THAT(inst, IsNull());
    }

    assertBasicBlockEquals(*basicBlocks[0], "\tINC foo\n");
    EXPECT_THAT(basicBlocks[0]->terminates(), Eq(true));
    assertBasicBlockEquals(*basicBlocks[1], "label:\n\tINC foo\n\tDEC foo\n\tGOTO label\n");
    EXPECT_THAT(basicBlocks[1]->terminates(), Eq(true));
    assertBasicBlockEquals(*basicBlocks[2], "\tDEC foo\n\tDEC foo\n");
    EXPECT_THAT(basicBlocks[2]->terminates(), Eq(true));
    assertBasicBlockEquals(*basicBlocks[3], "label:\n\tDEC foo\n\tDEC foo\n\tJA label\n");
    EXPECT_THAT(basicBlocks[3]->terminates(), Eq(true));
    assertBasicBlockEquals(*basicBlocks[4], "\tINC foo\n\tINC foo\n\tINC foo\n");
    EXPECT_THAT(basicBlocks[4]->terminates(), Eq(false));

    basicBlocks[4]->appendInstruction(std::make_unique<VoidReturn>());
    assertBasicBlockEquals(*basicBlocks[4], "\tINC foo\n\tINC foo\n\tINC foo\n\tRETURN\n");
    EXPECT_THAT(basicBlocks[4]->terminates(), Eq(true));
}

TEST(BasicBlock, identifiesSingleBasicBlock) {
    std::vector<std::unique_ptr<Quadruple>> instructions{};
    instructions.push_back(std::make_unique<VoidReturn>());

    auto basicBlocks = toBasicBlocks(std::move(instructions));

    EXPECT_THAT(basicBlocks, SizeIs(1));

    assertBasicBlockEquals(*basicBlocks[0], "\tRETURN\n");
    EXPECT_THAT(basicBlocks[0]->terminates(), Eq(true));
}

TEST(BasicBlock, identifiesEmptyBasicBlock) {
    std::vector<std::unique_ptr<Quadruple>> instructions{};

    auto basicBlocks = toBasicBlocks(std::move(instructions));

    EXPECT_THAT(basicBlocks, SizeIs(1));

    assertBasicBlockEquals(*basicBlocks[0], "");
    EXPECT_THAT(basicBlocks[0]->terminates(), Eq(false));
    basicBlocks[0]->appendInstruction(std::make_unique<VoidReturn>());
    assertBasicBlockEquals(*basicBlocks[0], "\tRETURN\n");
    EXPECT_THAT(basicBlocks[0]->terminates(), Eq(true));
}

}
