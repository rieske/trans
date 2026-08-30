#include "gtest/gtest.h"

#include "ast/AbstractSyntaxTree.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/Identifier.h"
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
