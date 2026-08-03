#include "DeclarationAnalyzer.h"

#include "AggregateInitSink.h"
#include "ArrayDecay.h"
#include "ConstantAddress.h"
#include "Conversion.h"
#include "InitializerLowering.h"
#include "ProductAssign.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/ArrayDeclarator.h"
#include "ast/EffectiveInitializer.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

// Object/typedef arrays must have a non-negative ICE bound. Parameter VLAs
// stay Unfixed and decay in getFundamentalType; they do not come through here.
static void checkObjectArrayBounds(ast::InitializedDeclarator& declarator,
        SemanticDiagnostics& diagnostics,
        ast::AbstractSyntaxTreeVisitor& visitor,
        bool allowVla) {
    bool tooLarge = false;
    bool negative = false;
    bool unfixed = false;
    declarator.forEachArrayDeclarator([&](ast::ArrayDeclarator& array) {
        if (!array.subscriptExpression || array.hasArraySize()) {
            return;
        }
        array.subscriptExpression->accept(visitor);
        switch (array.foldOwnBound()) {
        case ast::ArrayBoundFold::TooLarge:
            tooLarge = true;
            break;
        case ast::ArrayBoundFold::Negative:
            negative = true;
            break;
        case ast::ArrayBoundFold::Unfixed:
            unfixed = true;
            break;
        case ast::ArrayBoundFold::Complete:
            break;
        }
    });
    if (tooLarge) {
        diagnostics.error("array size is too large", declarator.getContext());
        return;
    }
    if (negative) {
        diagnostics.error("array size is negative", declarator.getContext());
        return;
    }
    if (unfixed && !allowVla) {
        diagnostics.error("array size is not a non-negative constant expression",
                declarator.getContext());
    }
}

// C allows refining an empty variadic prototype (int f(...)) with a more specific
// fixed-arg list (int f(const char*, ...)), and identical redeclarations.
// Top-level const/volatile on parameters is ignored for compatibility (C 6.7.6.3).
bool functionTypesCompatible(const type::Function& existing, const type::Function& incoming) {
    // Structural type equality (ignores top-level const/volatile on params/return).
    if (existing.getReturnType().equivalentTo(incoming.getReturnType())) {
        const auto existingArgs = existing.getArguments();
        const auto newArgs = incoming.getArguments();
        if (existingArgs.size() == newArgs.size()) {
            bool argsMatch = true;
            for (std::size_t i = 0; i < existingArgs.size(); ++i) {
                if (!existingArgs[i].equivalentTo(newArgs[i])) {
                    argsMatch = false;
                    break;
                }
            }
            if (argsMatch && existing.isVariadic() == incoming.isVariadic()) {
                return true;
            }
        }
    }
    const auto existingArgs = existing.getArguments();
    const auto newArgs = incoming.getArguments();
    // Builtin empty-vararg (0 fixed args) may be refined by a header prototype with
    // fixed parameters + "...", and vice versa for identical linkage.
    if (existing.isVariadic() && incoming.isVariadic()
            && (existingArgs.empty() || newArgs.empty())
            && existing.getReturnType().equivalentTo(incoming.getReturnType())) {
        return true;
    }
    return false;
}

static bool staticFollowsNonStatic(bool existingInternal, bool incomingInternal) {
    return !existingInternal && incomingInternal;
}

static std::string staticFollowsNonStaticMessage(const std::string& name) {
    return "static declaration of `" + name + "` follows non-static declaration";
}

static std::string nonStaticFollowsStaticMessage(const std::string& name) {
    return "non-static declaration of `" + name + "` follows static declaration";
}

static void reportObjectConflict(SemanticDiagnostics& diagnostics, SymbolTable& symbols,
        const std::string& name, const translation_unit::Context& context) {
    if (symbols.hasFunction(name)) {
        diagnostics.error("symbol `" + name + "` declaration conflicts with function of the same name",
                context);
        return;
    }
    std::string message = "symbol `" + name + "` declaration conflicts with previous declaration";
    if (symbols.hasSymbol(name)) {
        message += " on " + to_string(symbols.lookup(name).getContext());
    }
    diagnostics.error(std::move(message), context);
}

static bool bindOrInsertObject(SymbolTable& symbols, SemanticDiagnostics& diagnostics,
        const std::string& name, const type::Type& type, const translation_unit::Context& context,
        symbols::Storage storage, bool fileScope, bool hasInitializer) {
    if (fileScope) {
        switch (symbols.bindFileScopeObject(name, type, context, storage, hasInitializer)) {
        case ObjectBind::Bound:
            return true;
        case ObjectBind::StaticAfterNonStatic:
            diagnostics.error(staticFollowsNonStaticMessage(name), context);
            return false;
        case ObjectBind::NonStaticAfterStatic:
            diagnostics.error(nonStaticFollowsStaticMessage(name), context);
            return false;
        case ObjectBind::TypeConflict:
        case ObjectBind::SecondDefinition:
            reportObjectConflict(diagnostics, symbols, name, context);
            return false;
        }
        return false;
    }
    if (!symbols.insertSymbol(name, type, context, storage)) {
        reportObjectConflict(diagnostics, symbols, name, context);
        return false;
    }
    return true;
}

bool declareFunction(SymbolTable& symbols,
        const std::string& name,
        const type::Function& type,
        const translation_unit::Context& context,
        bool internalLinkage,
        FunctionDeclareKind kind,
        std::string& error) {
    if (symbols.hasEnumConstant(name) && symbols.isAtFileScope()) {
        error = "redefinition of enumerator `" + name + "` as a function";
        return false;
    }
    if (symbols.hasGlobalVariable(name)) {
        error = "function `" + name + "` conflicts with global variable of the same name";
        return false;
    }
    if (symbols.hasFunction(name)) {
        auto existing = symbols.findFunction(name);
        if (kind == FunctionDeclareKind::Definition && existing.isDefined()) {
            error = "function `" + name + "` definition conflicts with previous one on "
                    + to_string(existing.getContext());
            return false;
        }
        const char* what = kind == FunctionDeclareKind::Definition ? "definition" : "declaration";
        if (!functionTypesCompatible(existing.getType(), type)) {
            error = "function `" + name + "` " + what + " conflicts with previous one on "
                    + to_string(existing.getContext());
            return false;
        }
        if (staticFollowsNonStatic(existing.hasInternalLinkage(), internalLinkage)) {
            error = staticFollowsNonStaticMessage(name);
            return false;
        }
        if (kind == FunctionDeclareKind::Definition) {
            symbols.updateFunction(name, type, context);
        }
    } else {
        symbols.insertFunction(name, type, context, internalLinkage);
    }
    if (kind == FunctionDeclareKind::Definition) {
        symbols.markFunctionDefined(name);
    }
    return true;
}

DeclarationAnalyzer::DeclarationAnalyzer(SymbolTable& symbols,
        SemanticDiagnostics diagnostics,
        ast::AbstractSyntaxTreeVisitor& visitor,
        symbols::AnnotationStore& store) :
        symbolTable { symbols },
        diagnostics { std::move(diagnostics) },
        visitor { visitor },
        store { store }
{
}

void DeclarationAnalyzer::analyze(ast::Declaration& declaration) {
    auto declSpecs = declaration.getDeclarationSpecifiers();
    declSpecs.accept(visitor);

    // typedef only introduces a type alias; no runtime symbol.
    if (declSpecs.isTypedef()) {
        // Still visit declarators so nested constructs are analyzed if needed.
        for (const auto& declarator : declaration.getDeclarators()) {
            declarator->accept(visitor);
            checkObjectArrayBounds(*declarator, diagnostics, visitor, !symbolTable.isAtFileScope());
        }
        return;
    }

    auto baseType = declSpecs.getResolvedType();
    const bool isExtern = declSpecs.hasStorage(ast::Storage::EXTERN);
    const bool isStatic = declSpecs.hasStorage(ast::Storage::STATIC);
    for (const auto& declarator : declaration.getDeclarators()) {
        declarator->accept(visitor);
    }

    for (const auto& declarator : declaration.getDeclarators()) {
        analyzeInitializedDeclarator(*declarator, baseType, isExtern, isStatic);
    }
}

namespace {

symbols::Storage fileScopeObjectStorage(bool isExtern, bool isStatic, bool hasInit) {
    if (isStatic) {
        return symbols::Storage::Static;
    }
    if (isExtern && !hasInit) {
        return symbols::Storage::Extern;
    }
    return symbols::Storage::Global;
}

} // namespace

void DeclarationAnalyzer::analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
        const type::Type& baseType,
        bool isExtern,
        bool isStatic) {
    type::Type type { type::voidType() };
    try {
        type = declarator.getFundamentalType(baseType);
    } catch (const std::invalid_argument& ex) {
        diagnostics.error(ex.what(), declarator.getContext());
        return;
    }
    // Char-array string packing is sink-only (placeStringArray in lowerToFieldInits);
    // no AST rewrite to a brace list of byte constants.
    bool initializerVisited = false;
    if (type.isIncompleteArray() && declarator.hasInitializer()) {
        declarator.getInitializer()->accept(visitor);
        initializerVisited = true;
    }
    if (auto err = completeIncompleteArrayFromInitializer(type, declarator.getInitializer())) {
        diagnostics.error(std::move(*err), declarator.getContext());
        return;
    }
    if (type.isFunction()) {
        std::string error;
        if (!declareFunction(symbolTable, declarator.getName(), type.getFunction(),
                declarator.getContext(), isStatic, FunctionDeclareKind::Prototype, error)) {
            diagnostics.error(std::move(error), declarator.getContext());
        }
        return;
    }
    if (type.isVoid()) {
        diagnostics.error("variable `" + declarator.getName() + "` declared void", declarator.getContext());
        return;
    }
    const bool fileScope = symbolTable.isAtFileScope();
    if (type::isIncompleteObjectType(type) && !(isExtern && !declarator.hasInitializer())) {
        diagnostics.error("variable `" + declarator.getName() + "` has incomplete type",
                declarator.getContext());
        return;
    }
    if (fileScope && symbolTable.hasEnumConstant(declarator.getName())) {
        diagnostics.error("redefinition of enumerator `" + declarator.getName() + "`",
                declarator.getContext());
        return;
    }
    symbols::Storage storage = symbols::Storage::Automatic;
    if (fileScope) {
        storage = fileScopeObjectStorage(isExtern, isStatic, declarator.hasInitializer());
    } else if (isExtern && !declarator.hasInitializer()) {
        storage = symbols::Storage::Extern;
    } else if (isStatic) {
        storage = symbols::Storage::Static;
    }
    checkObjectArrayBounds(declarator, diagnostics, visitor, storage == symbols::Storage::Automatic);
    if (!bindOrInsertObject(symbolTable, diagnostics, declarator.getName(), type,
            declarator.getContext(), storage, fileScope, declarator.hasInitializer())) {
        return;
    }
    // C 6.2.1: scope begins after the declarator completes, so the name is in
    // scope for the initializer (sizeof(*p), value self-ref, &p, etc.).
    store.setValue(&declarator, symbols::ValueSlot::Holder, symbolTable.lookup(declarator.getName()));
    symbols::ValueEntry* holder = store.value(&declarator, symbols::ValueSlot::Holder);
    ast::Expression* initAst = nullptr;
    ast::Expression* effectiveInit = nullptr;
    if (declarator.hasInitializer()) {
        initAst = declarator.getInitializer();
        if (!initializerVisited) {
            initAst->accept(visitor);
        }
        const translation_unit::Context initCtx = declarator.getContext();
        std::string excessError;
        effectiveInit = ast::effectiveInitializer(type, initAst, &excessError);
        if (!excessError.empty()) {
            diagnostics.error(std::move(excessError), initCtx);
        }
        const ObjectInitKind initKind = classifyObjectInit(type, initAst);
        if (dynamic_cast<ast::StringLiteralExpression*>(effectiveInit) && !type.isPointer()
                && initKind != ObjectInitKind::CharArrayString) {
            diagnostics.error("string literal initializer requires pointer type",
                    declarator.getContext());
            return;
        }
        const bool scalarAssign = initKind == ObjectInitKind::Scalar;
        if (scalarAssign && effectiveInit && effectiveInit->hasResultSymbol(store)) {
            const type::Type& from = effectiveInit->getResultSymbol(store)->getType();
            if (!reportProductAssign(
                    [&](std::string msg, const translation_unit::Context& c) {
                        diagnostics.error(std::move(msg), c);
                    },
                    type, from, declarator.getContext(), effectiveInit)) {
                return;
            }
        }
        if (effectiveInit && holder->isGlobal()) {
            applyGlobalInitializer(declarator, type, initAst, effectiveInit);
        }
        if (type.isPointer()) {
            decayArrayInPlace(effectiveInit, symbolTable, store);
        }
        if (scalarAssign && effectiveInit && effectiveInit->hasResultSymbol(store)) {
            maybeSetConversion(effectiveInit, type, symbolTable, store);
        }
    }
    if (effectiveInit && !fileScope && holder && !holder->isGlobal()
            && holder->getType().isAggregate()) {
        applyLocalAggregateInitializer(declarator, initAst);
    }
}

void DeclarationAnalyzer::applyGlobalInitializer(ast::InitializedDeclarator& declarator,
        type::Type& type,
        ast::Expression* initAst,
        ast::Expression* effectiveInit) {
    const std::string storageName = declarator.getName();
    symbols::GlobalInitializer init;
    if (tryFoldGlobalInit(effectiveInit, type, store, init)) {
        symbolTable.setGlobalInitializer(storageName, std::move(init));
        return;
    }
    // Aggregate brace lists and char[N] = "..." / { "..." } share lowerToDataWords.
    if (type.isAggregate()) {
        type::Type objectType = type;
        type::Type completedType = type;
        std::vector<symbols::DataWord> words;
        AggregateInitHost host {
            store,
            [this](std::string msg, const translation_unit::Context& ctx) {
                diagnostics.error(std::move(msg), ctx);
            }
        };
        const DataWordsLowering lowered =
                lowerToDataWords(objectType, initAst, host, completedType, words);
        if (lowered == DataWordsLowering::Ok) {
            if (completedType.getSize() != type.getSize()) {
                symbolTable.setType(declarator.getName(), completedType);
                store.setValue(&declarator, symbols::ValueSlot::Holder,
                        symbolTable.lookup(declarator.getName()));
                type = completedType;
            }
            symbolTable.setGlobalInitializer(storageName, symbols::MultiWordInit { std::move(words) });
            return;
        }
        if (lowered == DataWordsLowering::Failed) {
            // Sink already reported (excess string, non-constant brace leaf, ...).
            return;
        }
    }
    diagnostics.error("global initializer is not a constant expression", declarator.getContext());
}

void DeclarationAnalyzer::applyLocalAggregateInitializer(ast::InitializedDeclarator& declarator,
        ast::Expression* initAst) {
    type::Type holderType = store.value(&declarator, symbols::ValueSlot::Holder)->getType();
    AggregateInitHost host {
        store,
        [this](std::string msg, const translation_unit::Context& ctx) {
            diagnostics.error(std::move(msg), ctx);
        }
    };
    for (auto& init : lowerToFieldInits(holderType, initAst, symbolTable, host)) {
        store.addFieldInit(&declarator, std::move(init));
    }
}

} // namespace semantic_analyzer
