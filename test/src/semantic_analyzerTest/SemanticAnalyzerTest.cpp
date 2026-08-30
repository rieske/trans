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
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"

#include <memory>
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
    EXPECT_THROW(analyzer.analyze(*tree, session), std::runtime_error);
}

TEST(SemanticAnalyzer, fileScopeObjectWithoutSessionEnumeratorIsOk) {
    scanner::LexicalSession session;

    auto tree = fileScopeInt("E");
    semantic_analyzer::SemanticAnalyzer analyzer { false };
    analyzer.analyze(*tree, session);
}
