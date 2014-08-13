#ifndef _CONTEXTUAL_SYNTAX_NODE_BUILDER_
#define _CONTEXTUAL_SYNTAX_NODE_BUILDER_

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"

namespace semantic_analyzer {

class AbstractSyntaxTreeBuilderContext;
class AbstractSyntaxTreeNode;

class ContextualSyntaxNodeBuilder {
public:
    ContextualSyntaxNodeBuilder();
    virtual ~ContextualSyntaxNodeBuilder();

    void updateContext(std::string definingSymbol, const std::vector<std::string>& production,
            AbstractSyntaxTreeBuilderContext& context) const;

private:
    static void noCreatorDefined(std::string definingSymbol, const std::vector<std::string>& production);
    static void doNothing(AbstractSyntaxTreeBuilderContext&);

    static void parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context);
    static void term(AbstractSyntaxTreeBuilderContext& context);

    static void arrayAccess(AbstractSyntaxTreeBuilderContext& context);
    static void functionCall(AbstractSyntaxTreeBuilderContext& context);
    static void noargFunctionCall(AbstractSyntaxTreeBuilderContext& context);
    static void postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context);

    static void prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context);
    static void unaryExpression(AbstractSyntaxTreeBuilderContext& context);

    static void typeCast(AbstractSyntaxTreeBuilderContext& context);
    static void pointerCast(AbstractSyntaxTreeBuilderContext& context);

    static void arithmeticExpression(AbstractSyntaxTreeBuilderContext& context);
    static void shiftExpression(AbstractSyntaxTreeBuilderContext& context);
    static void comparisonExpression(AbstractSyntaxTreeBuilderContext& context);
    static void bitwiseExpression(AbstractSyntaxTreeBuilderContext& context);
    static void logicalAndExpression(AbstractSyntaxTreeBuilderContext& context);
    static void logicalOrExpression(AbstractSyntaxTreeBuilderContext& context);
    static void assignmentExpression(AbstractSyntaxTreeBuilderContext& context);
    static void expressionList(AbstractSyntaxTreeBuilderContext& context);

    static void createAssignmentExpressionList(AbstractSyntaxTreeBuilderContext& context);
    static void addAssignmentExpressionToList(AbstractSyntaxTreeBuilderContext& context);

    static void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);
    static void returnStatement(AbstractSyntaxTreeBuilderContext& context);
    static void inputOutputStatement(AbstractSyntaxTreeBuilderContext& context);

    static void whileLoopHeader(AbstractSyntaxTreeBuilderContext& context);
    static void forLoopHeader(AbstractSyntaxTreeBuilderContext& context);

    static void ifStatement(AbstractSyntaxTreeBuilderContext& context);
    static void ifElseStatement(AbstractSyntaxTreeBuilderContext& context);
    static void loopStatement(AbstractSyntaxTreeBuilderContext& context);
    static void emptyStatement(AbstractSyntaxTreeBuilderContext& context);
    static void expressionStatement(AbstractSyntaxTreeBuilderContext& context);

    static void parameterDeclaration(AbstractSyntaxTreeBuilderContext& context);
    static void parameterList(AbstractSyntaxTreeBuilderContext& context);
    static void addParameterToList(AbstractSyntaxTreeBuilderContext& context);

    static void identifierDeclaration(AbstractSyntaxTreeBuilderContext& context);
    static void functionDeclaration(AbstractSyntaxTreeBuilderContext& context);
    static void noargFunctionDeclaration(AbstractSyntaxTreeBuilderContext& context);
    static void arrayDeclaration(AbstractSyntaxTreeBuilderContext& context);

    static void pointer(AbstractSyntaxTreeBuilderContext& context);
    static void pointerToPointer(AbstractSyntaxTreeBuilderContext& context);

    static void singleBlock(AbstractSyntaxTreeBuilderContext& context);
    static void doubleBlock(AbstractSyntaxTreeBuilderContext& context);

    static void pointerToDeclaration(AbstractSyntaxTreeBuilderContext& context);
    static void declarationList(AbstractSyntaxTreeBuilderContext& context);
    static void addDeclarationToList(AbstractSyntaxTreeBuilderContext& context);

    static void variableDeclaration(AbstractSyntaxTreeBuilderContext& context);
    static void variableDefinition(AbstractSyntaxTreeBuilderContext& context);

    static void functionDefinition(AbstractSyntaxTreeBuilderContext& context);

    static void newListCarrier(AbstractSyntaxTreeBuilderContext& context);
    static void addToListCarrier(AbstractSyntaxTreeBuilderContext& context);

    static void functionsTranslationUnit(AbstractSyntaxTreeBuilderContext& context);
    static void variablesFunctionsTranslationUnit(AbstractSyntaxTreeBuilderContext& context);

    static void newIntegerType(AbstractSyntaxTreeBuilderContext& context);
    static void newCharacterType(AbstractSyntaxTreeBuilderContext& context);
    static void newVoidType(AbstractSyntaxTreeBuilderContext& context);
    static void newFloatType(AbstractSyntaxTreeBuilderContext& context);

    std::unordered_map<std::string, std::map<std::vector<std::string>, std::function<void(AbstractSyntaxTreeBuilderContext&)>>>nodeCreatorRegistry;
};

}
/* namespace semantic_analyzer */

#endif /* _CONTEXTUAL_SYNTAX_NODE_BUILDER_ */
