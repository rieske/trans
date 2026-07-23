#include "CSNB_Internal.h"

#include "CompoundLiteralExpression.h"
#include "Declaration.h"
#include "InitializedDeclarator.h"
#include "InitializerListExpression.h"
#include "scanner/FiniteAutomaton.h"
#include "scanner/TypedefRegistry.h"
#include "types/Type.h"

namespace ast {
namespace csnb {

namespace {

void compoundLiteralExpression(AbstractSyntaxTreeBuilderContext& context) {
    // ( type_name ) { initializer_list }  or with trailing comma
    context.popTerminal(); // }
    // Optional trailing comma is a separate production that pops it before this path
    // is registered; both productions share this builder after terminals are aligned.
    auto initExpr = context.popExpression();
    context.popTerminal(); // {
    context.popTerminal(); // )
    auto typeSpec = context.popTypeSpecifier();
    if (typeSpec.getName() == "__typeof__" && context.hasTypeofOperand()) {
        (void)context.popTypeofOperand();
    }
    auto open = context.popTerminal(); // (
    auto* list = dynamic_cast<InitializerListExpression*>(initExpr.get());
    if (!list) {
        // Single expression was reduced as initializer -> wrap.
        std::vector<InitializerElement> elements;
        elements.emplace_back(std::move(initExpr));
        context.pushExpression(std::make_unique<CompoundLiteralExpression>(
                typeSpec,
                std::make_unique<InitializerListExpression>(std::move(elements)),
                open.context));
        return;
    }
    initExpr.release();
    context.pushExpression(std::make_unique<CompoundLiteralExpression>(
            typeSpec,
            std::unique_ptr<InitializerListExpression>(list),
            open.context));
}

void compoundLiteralExpressionTrailingComma(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // ,
    auto initExpr = context.popExpression();
    context.popTerminal(); // {
    context.popTerminal(); // )
    auto typeSpec = context.popTypeSpecifier();
    if (typeSpec.getName() == "__typeof__" && context.hasTypeofOperand()) {
        (void)context.popTypeofOperand();
    }
    auto open = context.popTerminal(); // (
    auto* list = dynamic_cast<InitializerListExpression*>(initExpr.get());
    if (!list) {
        std::vector<InitializerElement> elements;
        elements.emplace_back(std::move(initExpr));
        context.pushExpression(std::make_unique<CompoundLiteralExpression>(
                typeSpec,
                std::make_unique<InitializerListExpression>(std::move(elements)),
                open.context));
        return;
    }
    initExpr.release();
    context.pushExpression(std::make_unique<CompoundLiteralExpression>(
            typeSpec,
            std::unique_ptr<InitializerListExpression>(list),
            open.context));
}
void singleElementInitializerList(AbstractSyntaxTreeBuilderContext& context) {
    auto value = context.popExpression();
    InitializerElement element { std::move(value) };
    std::vector<InitializerElement> elements;
    elements.push_back(std::move(element));
    context.pushExpression(std::make_unique<InitializerListExpression>(std::move(elements)));
}

void designatedSingleElementInitializerList(AbstractSyntaxTreeBuilderContext& context) {
    auto value = context.popExpression();
    InitializerElement element { std::move(value) };
    context.takePendingDesignator(element.memberPath, element.arrayIndex);
    std::vector<InitializerElement> elements;
    elements.push_back(std::move(element));
    context.pushExpression(std::make_unique<InitializerListExpression>(std::move(elements)));
}

void addToInitializerList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ,
    auto value = context.popExpression();
    InitializerElement element { std::move(value) };
    auto listExpression = context.popExpression();
    auto* list = dynamic_cast<InitializerListExpression*>(listExpression.get());
    if (!list) {
        throw std::runtime_error { "expected initializer list while appending element" };
    }
    list->appendElement(std::move(element));
    context.pushExpression(std::move(listExpression));
}

void addDesignatedToInitializerList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ,
    auto value = context.popExpression();
    InitializerElement element { std::move(value) };
    context.takePendingDesignator(element.memberPath, element.arrayIndex);
    auto listExpression = context.popExpression();
    auto* list = dynamic_cast<InitializerListExpression*>(listExpression.get());
    if (!list) {
        throw std::runtime_error { "expected initializer list while appending designated element" };
    }
    list->appendElement(std::move(element));
    context.pushExpression(std::move(listExpression));
}

void memberDesignator(AbstractSyntaxTreeBuilderContext& context) {
    auto member = context.popTerminal(); // id
    context.popTerminal(); // .
    context.pushMemberDesignator(member.value);
}

void arrayDesignator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ]
    auto indexExpression = context.popExpression();
    context.popTerminal(); // [
    context.pushArrayIndexDesignator(std::move(indexExpression));
}

void designatorListSingle(AbstractSyntaxTreeBuilderContext& context) {
    (void)context;
}

void designatorListAppend(AbstractSyntaxTreeBuilderContext& context) {
    // Nested designators (e.g. .a.b or .pruning.flags.recursive): each
    // designator is pushed as it reduces; merge newest onto the base path
    // (git REV_INFO_INIT). Stack top is the newest segment.
    std::vector<std::string> suffix;
    std::optional<long> suffixIndex;
    context.takePendingDesignator(suffix, suffixIndex);
    std::vector<std::string> basePath;
    std::optional<long> baseIndex;
    context.takePendingDesignator(basePath, baseIndex);
    for (auto& name : suffix) {
        basePath.push_back(std::move(name));
    }
    if (suffixIndex) {
        baseIndex = suffixIndex;
    }
    context.pushPendingDesignator(std::move(basePath), baseIndex);
}

void designation(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // =
}

void braceInitializer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // {
    // Initializer list expression remains on the expression stack.
}

void braceInitializerTrailingComma(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // ,
    context.popTerminal(); // {
}

void initializedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator)));
}

void initializedDeclaratorWithInitializer(AbstractSyntaxTreeBuilderContext& context) {
    // Production: <declarator> '=' <initializer> - must consume '=' or it stays on the
    // terminal stack and poisons later reductions (e.g. labeled for-with-decl sees
    // label name as ":" because '=' displaces the real punctuation pops).
    context.popTerminal(); // =
    auto declarator = context.popDeclarator();
    auto initializerExpression = context.popExpression();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator), std::move(initializerExpression)));
}

void initializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<std::unique_ptr<InitializedDeclarator>> declarators;
    declarators.push_back(context.popInitializedDeclarator());
    context.pushInitializedDeclarators(std::move(declarators));
}

void addToInitializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto initializedDeclarators = context.popInitializedDeclarators();
    initializedDeclarators.push_back(context.popInitializedDeclarator());
    context.pushInitializedDeclarators(std::move(initializedDeclarators));
}

void initializedDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto initializedDeclarators = context.popInitializedDeclarators();

    // Register typedef names with the lexer and type map as soon as the decl is reduced,
    // so subsequent tokens in the same translation unit can use the new type name.
    bool isTypedef = false;
    for (const auto& storage : declarationSpecifiers.getStorageSpecifiers()) {
        if (storage.getStorage() == Storage::TYPEDEF) {
            isTypedef = true;
            break;
        }
    }
    if (isTypedef && !declarationSpecifiers.getTypeSpecifiers().empty()) {
        auto baseType = declarationSpecifiers.getResolvedType();
        for (const auto& declarator : initializedDeclarators) {
            type::Type aliased = declarator->getFundamentalType(baseType);
            context.environment().defineTypedef(declarator->getName(), aliased);
            scanner::FiniteAutomaton::registerTypedefName(declarator->getName());
            scanner::TypedefRegistry::add(declarator->getName(), aliased);
        }
    } else if (!isTypedef) {
        // Object declarator reusing a typedef name (git: cmp_type cmp_type) shadows the
        // typedef for later tokens in the same brace scope.
        for (const auto& declarator : initializedDeclarators) {
            const std::string& name = declarator->getName();
            if (scanner::TypedefRegistry::has(name)) {
                scanner::TypedefRegistry::addIdentifierShadow(name);
            }
        }
    }

    context.pushDeclaration(std::make_unique<Declaration>(declarationSpecifiers, std::move(initializedDeclarators)));
}

void declaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    context.pushDeclaration(std::make_unique<Declaration>(declarationSpecifiers));
}

void declarationList(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<std::unique_ptr<Declaration>> declarations;
    declarations.push_back(context.popDeclaration());
    context.pushDeclarationList(std::move(declarations));
}

void addDeclarationToList(AbstractSyntaxTreeBuilderContext& context) {
    auto declarations = context.popDeclarationList();
    declarations.push_back(context.popDeclaration());
    context.pushDeclarationList(std::move(declarations));
}

} // namespace


void registerInitializerProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_comma = grammar.symbolId(",");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_postfix_exp = grammar.symbolId("<postfix_exp>");
    int s_assignment = grammar.symbolId("<assignment_exp>");
    int s_initializer = grammar.symbolId("<initializer>");
    int s_initializer_list = grammar.symbolId("<initializer_list>");
    int s_designator = grammar.symbolId("<designator>");
    int s_designator_list = grammar.symbolId("<designator_list>");
    int s_designation = grammar.symbolId("<designation>");
    int s_open_brace = grammar.symbolId("{");
    int s_close_brace = grammar.symbolId("}");
    int s_init_open_bracket = grammar.symbolId("[");
    int s_init_close_bracket = grammar.symbolId("]");
    int s_const_exp_init = grammar.symbolId("<const_exp>");
    int s_declarator = grammar.symbolId("<declarator>");
    int s_decl_specs = grammar.symbolId("<decl_specs>");
    int s_semicolon = grammar.symbolId(";");

    // C99 compound literals: (type){ init } as postfix expressions.
    nodeCreatorRegistry[s_postfix_exp][{
            s_open_paren, s_type_name, s_close_paren, s_open_brace, s_initializer_list,
            s_close_brace }] = compoundLiteralExpression;
    nodeCreatorRegistry[s_postfix_exp][{
            s_open_paren, s_type_name, s_close_paren, s_open_brace, s_initializer_list,
            s_comma, s_close_brace }] = compoundLiteralExpressionTrailingComma;
    nodeCreatorRegistry[s_initializer][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_initializer][{ s_open_brace, s_initializer_list, s_close_brace }] = braceInitializer;
    nodeCreatorRegistry[s_initializer][{ s_open_brace, s_initializer_list, s_comma, s_close_brace }] = braceInitializerTrailingComma;

    nodeCreatorRegistry[s_designator][{ grammar.symbolId("."), grammar.symbolId("id") }] = memberDesignator;
    nodeCreatorRegistry[s_designator][{ s_init_open_bracket, s_const_exp_init, s_init_close_bracket }] = arrayDesignator;
    nodeCreatorRegistry[s_designator_list][{ s_designator }] = designatorListSingle;
    nodeCreatorRegistry[s_designator_list][{ s_designator_list, s_designator }] = designatorListAppend;
    nodeCreatorRegistry[s_designation][{ s_designator_list, grammar.symbolId("=") }] = designation;

    nodeCreatorRegistry[s_initializer_list][{ s_initializer }] = singleElementInitializerList;
    nodeCreatorRegistry[s_initializer_list][{ s_designation, s_initializer }] = designatedSingleElementInitializerList;
    nodeCreatorRegistry[s_initializer_list][{ s_initializer_list, s_comma, s_initializer }] = addToInitializerList;
    nodeCreatorRegistry[s_initializer_list][{ s_initializer_list, s_comma, s_designation, s_initializer }] =
            addDesignatedToInitializerList;

    int s_init_declarator = grammar.symbolId("<init_declarator>");
    nodeCreatorRegistry[s_init_declarator][{ s_declarator }] = initializedDeclarator;
    nodeCreatorRegistry[s_init_declarator][{ s_declarator, grammar.symbolId("="), s_initializer }] = initializedDeclaratorWithInitializer;

    int s_init_declarator_list = grammar.symbolId("<init_declarator_list>");
    nodeCreatorRegistry[s_init_declarator_list][{ s_init_declarator }] = initializedDeclaratorList;
    nodeCreatorRegistry[s_init_declarator_list][{ s_init_declarator_list, s_comma, s_init_declarator }] = addToInitializedDeclaratorList;

    int s_decl = grammar.symbolId("<decl>");
    nodeCreatorRegistry[s_decl][{ s_decl_specs, s_init_declarator_list, s_semicolon }] = initializedDeclaration;
    nodeCreatorRegistry[s_decl][{ s_decl_specs, s_semicolon }] = declaration;

    int s_decl_list = grammar.symbolId("<decl_list>");
    nodeCreatorRegistry[s_decl_list][{ s_decl }] = declarationList;
    nodeCreatorRegistry[s_decl_list][{ s_decl_list, s_decl }] = addDeclarationToList;
}

} // namespace csnb
} // namespace ast
