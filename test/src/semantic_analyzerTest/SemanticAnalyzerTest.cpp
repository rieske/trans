#include "gtest/gtest.h"

#include "ast/AbstractSyntaxTree.h"
#include "ast/Block.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/FormalArgument.h"
#include "ast/FunctionDeclarator.h"
#include "ast/FunctionDefinition.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "scanner/LexicalSession.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "util/Diagnostic.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

using namespace ast;

translation_unit::Context ctx() {
    return { "t.c", 1 };
}

DeclarationSpecifiers intSpecs() {
    return DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } };
}

std::unique_ptr<AbstractSyntaxTree> fileScopeInt(const std::string& name) {
    std::vector<std::unique_ptr<InitializedDeclarator>> declarators;
    TerminalSymbol id { "id", name, { "t.c", 1 } };
    declarators.push_back(std::make_unique<InitializedDeclarator>(
            std::make_unique<Declarator>(std::make_unique<Identifier>(id))));

    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
    translationUnit.push_back(std::make_unique<Declaration>(
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } },
            std::move(declarators)));
    auto tree = std::make_unique<AbstractSyntaxTree>(std::move(translationUnit));
    tree->setVlaExpressions(std::make_shared<VlaExpressionTable>());
    return tree;
}

} // namespace

TEST(SemanticAnalyzer, sessionEnumeratorConflictsWithFileScopeObject) {
    scanner::LexicalSession session;
    session.enums.add("E", type::fromHostLong(0));

    auto tree = fileScopeInt("E");
    semantic_analyzer::SemanticAnalyzer analyzer { false };
    std::ostringstream ignored;
    diag::Sink sink(ignored);
    EXPECT_FALSE(analyzer.analyze(*tree, session, sink));
    EXPECT_TRUE(sink.hasErrors());
}

TEST(SemanticAnalyzer, fileScopeObjectWithoutSessionEnumeratorIsOk) {
    scanner::LexicalSession session;

    auto tree = fileScopeInt("E");
    semantic_analyzer::SemanticAnalyzer analyzer { false };
    std::ostringstream ignored;
    diag::Sink sink(ignored);
    EXPECT_TRUE(analyzer.analyze(*tree, session, sink));
    EXPECT_FALSE(sink.hasErrors());
}

TEST(SemanticAnalyzer, missingSessionIsInternalError) {
    auto tree = fileScopeInt("E");
    semantic_analyzer::SemanticAnalysisVisitor visitor;
    visitor.setGnuExtensions(false);
    visitor.setAnnotationStore(tree->annotations());
    visitor.setVlaExpressions(tree->vlaExpressions());
    EXPECT_THROW((*tree->begin())->accept(visitor), std::logic_error);
}

TEST(SemanticAnalyzer, functionDesignatorKeepsVariadic) {
    FormalArguments args;
    args.push_back(FormalArgument {
            intSpecs(), std::make_unique<Declarator>(std::make_unique<Identifier>(
                    TerminalSymbol { "id", "x", ctx() })) });
    auto fn = std::make_unique<FunctionDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "f", ctx() }),
            std::move(args), true);

    auto designator = std::make_unique<IdentifierExpression>("f", ctx());
    auto* used = designator.get();
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> bodyItems;
    bodyItems.push_back(std::move(designator));

    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> translationUnit;
    translationUnit.push_back(std::make_unique<FunctionDefinition>(
            intSpecs(),
            std::make_unique<Declarator>(std::move(fn)),
            std::make_unique<Block>(std::move(bodyItems))));
    auto tree = std::make_unique<AbstractSyntaxTree>(std::move(translationUnit));
    tree->setVlaExpressions(std::make_shared<VlaExpressionTable>());

    scanner::LexicalSession session;
    semantic_analyzer::SemanticAnalyzer analyzer { false };
    std::ostringstream ignored;
    diag::Sink sink(ignored);
    ASSERT_TRUE(analyzer.analyze(*tree, session, sink)) << ignored.str();
    ASSERT_TRUE(used->holdsFunctionDesignator());
    ASSERT_TRUE(used->expressionType().isFunction());
    EXPECT_TRUE(used->expressionType().getFunction().isVariadic());
}
