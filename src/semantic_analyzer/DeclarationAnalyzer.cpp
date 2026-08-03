#include "DeclarationAnalyzer.h"

#include "AggregateInitSink.h"
#include "ConstantAddress.h"
#include "InitializerLowering.h"
#include "ArrayDecay.h"

#include <cstring>
#include <vector>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/ConstantExpression.h"
#include "ast/EffectiveInitializer.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "util/FloatingLiteral.h"
#include "util/StringLiteralDecode.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {


// Encode a double as IEEE-754 bit pattern stored in a long (for .data init).
static long doubleBitsAsLong(double d) {
    unsigned long long bits = 0;
    static_assert(sizeof(double) == sizeof(unsigned long long), "double must be 64-bit");
    std::memcpy(&bits, &d, sizeof(bits));
    return static_cast<long>(bits);
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
    bool isTypedef = false;
    for (const auto& storage : declSpecs.getStorageSpecifiers()) {
        if (storage.getStorage() == ast::Storage::TYPEDEF) {
            isTypedef = true;
            break;
        }
    }
    if (isTypedef) {
        // Still visit declarators so nested constructs are analyzed if needed.
        for (const auto& declarator : declaration.getDeclarators()) {
            declarator->accept(visitor);
        }
        return;
    }

    auto baseType = declSpecs.getResolvedType();
    bool isExtern = false;
    bool isStaticStorage = false;
    for (const auto& storage : declSpecs.getStorageSpecifiers()) {
        if (storage.getStorage() == ast::Storage::EXTERN) {
            isExtern = true;
        } else if (storage.getStorage() == ast::Storage::STATIC) {
            isStaticStorage = true;
        }
    }
    for (const auto& declarator : declaration.getDeclarators()) {
        declarator->accept(visitor);
    }

    for (const auto& declarator : declaration.getDeclarators()) {
        analyzeInitializedDeclarator(*declarator, baseType, isExtern, isStaticStorage);
    }
}

void DeclarationAnalyzer::analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
        const type::Type& baseType,
        bool isExtern,
        bool isStaticStorage) {
    auto type = declarator.getFundamentalType(baseType);
    if (type.isFunction()) {
        // Function prototype.
        if (symbolTable.hasGlobalVariable(declarator.getName())) {
            diagnostics.error("function `" + declarator.getName() + "` conflicts with global variable of the same name",
                    declarator.getContext());
            return;
        }
        if (symbolTable.hasFunction(declarator.getName())) {
            auto existing = symbolTable.findFunction(declarator.getName());
            if (!functionTypesCompatible(existing.getType(), type.getFunction())) {
                diagnostics.error("function `" + declarator.getName() + "` declaration conflicts with previous one on "
                        + to_string(existing.getContext()), declarator.getContext());
            }
        } else {
            symbolTable.insertFunction(declarator.getName(), type.getFunction(), declarator.getContext());
        }
        return;
    }
    if (type.isVoid()) {
        diagnostics.error("variable `" + declarator.getName() + "` declared void", declarator.getContext());
    } else if (symbolTable.isAtFileScope() && symbolTable.hasFunction(declarator.getName())) {
        diagnostics.error("symbol `" + declarator.getName() + "` declaration conflicts with function of the same name",
                declarator.getContext());
    } else if (isExtern && !symbolTable.isAtFileScope() && !declarator.hasInitializer()) {
        // Block-scope extern: external linkage, no automatic storage.
        if (!symbolTable.insertExternLocal(declarator.getName(), type, declarator.getContext())) {
            diagnostics.error("symbol `" + declarator.getName() + "` declaration conflicts with previous one",
                    declarator.getContext());
        } else {
            store.setValue(&declarator, symbols::ValueSlot::Holder, symbolTable.lookup(declarator.getName()));
        }
    } else if (isStaticStorage && !symbolTable.isAtFileScope()
            ? symbolTable.insertStaticLocal(declarator.getName(), type, declarator.getContext())
            : symbolTable.insertSymbol(declarator.getName(), type, declarator.getContext())) {
        // C 6.2.1: scope begins after the declarator completes, so the name is in
        // scope for the initializer (sizeof(*p), value self-ref, &p, etc.).
        store.setValue(&declarator, symbols::ValueSlot::Holder, symbolTable.lookup(declarator.getName()));
        if (symbolTable.isAtFileScope()) {
            // A non-extern declaration (or any with an initializer) is a definition:
            // clear any prior pure-extern mark from a header prototype.
            if (isExtern && !declarator.hasInitializer()) {
                symbolTable.setGlobalExternal(declarator.getName(), true);
            } else {
                symbolTable.setGlobalExternal(declarator.getName(), false);
            }
            if (isStaticStorage) {
                symbolTable.setGlobalStaticStorage(declarator.getName(), true);
            }
        }
        ast::Expression* initAst = nullptr;
        ast::Expression* effectiveInit = nullptr;
        if (declarator.hasInitializer()) {
            initAst = declarator.getInitializer();
            initAst->accept(visitor);
            // Single peel for type-check, .data, and (via CG helper) assign.
            const translation_unit::Context initCtx = declarator.getContext();
            std::string excessError;
            effectiveInit = ast::effectiveInitializer(type, initAst, &excessError);
            if (!excessError.empty()) {
                diagnostics.error(std::move(excessError), initCtx);
            }
            // T *p = arr; decay array initializer in place (same model as call/assign).
            if (type.isPointer()) {
                decayArrayInPlace(effectiveInit, symbolTable, store);
            }
            const bool aggregateBrace =
                    dynamic_cast<ast::InitializerListExpression*>(initAst) != nullptr
                    && type.isAggregate();
            const bool stringInit =
                    dynamic_cast<ast::StringLiteralExpression*>(effectiveInit) != nullptr;
            if (!aggregateBrace && !stringInit && effectiveInit && effectiveInit->hasResult(store)) {
                const type::Type& from = effectiveInit->result(store)->getType();
                // Holder type may still be incomplete array; use analyzed `type`.
                if (!type::productAssignFrom(type, from)) {
                    diagnostics.error(type::productAssignFailureMessage(type, from),
                            declarator.getContext());
                }
            }
        }

        if (effectiveInit && store.value(&declarator, symbols::ValueSlot::Holder)->isGlobal()) {
            applyGlobalInitializer(declarator, type, initAst, effectiveInit);
        } else if (effectiveInit && !symbolTable.isAtFileScope()
                && store.value(&declarator, symbols::ValueSlot::Holder)
                && store.value(&declarator, symbols::ValueSlot::Holder)->getType().isAggregate()) {
            applyLocalAggregateInitializer(declarator, initAst);
        }
    } else {
        diagnostics.error(
                "symbol `" + declarator.getName() +
                        "` declaration conflicts with previous declaration on " +
                        to_string(symbolTable.lookup(declarator.getName()).getContext()),
                declarator.getContext());
    }
}

void DeclarationAnalyzer::applyGlobalInitializer(ast::InitializedDeclarator& declarator,
        type::Type& type,
        ast::Expression* initAst,
        ast::Expression* effectiveInit) {
    const std::string storageName = store.value(&declarator, symbols::ValueSlot::Holder)->getName();
    long initValue = 0;
    if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(effectiveInit)) {
        if (type.isArray()) {
            // const char name[] = "..." : embed bytes; complete incomplete arrays (incl. NUL).
            if (type.getArraySize() == 0) {
                const int length = util::stringLiteralArrayLength(strLit->getValue());
                type = type::array(type.getElementType(), length);
                symbolTable.setGlobalType(storageName, type);
                symbolTable.setLocalType(declarator.getName(), type);
                store.setValue(&declarator, symbols::ValueSlot::Holder, symbolTable.lookup(declarator.getName()));
            }
            symbolTable.setGlobalStringInitializer(storageName, strLit->getValue());
        } else if (type.isPointer()) {
            symbolTable.setGlobalAddressInitializer(storageName, strLit->getConstantSymbol());
        } else {
            diagnostics.error("string literal initializer requires array or pointer type",
                    declarator.getContext());
        }
        return;
    }
    if (type::isFloating(type)) {
        if (auto* fconst = dynamic_cast<ast::ConstantExpression*>(effectiveInit)) {
            util::FloatingBits parsed;
            if (util::floatingLiteralBits(fconst->getValue(), parsed)) {
                symbolTable.setGlobalInitializer(storageName, static_cast<long>(parsed.bits));
            } else {
                diagnostics.error("global floating initializer is not a constant", declarator.getContext());
            }
        } else if (effectiveInit->evaluateConstant(initValue)) {
            if (type.getSize() == 4) {
                float f = static_cast<float>(initValue);
                unsigned bits32 = 0;
                static_assert(sizeof(float) == sizeof(unsigned), "unexpected float size");
                std::memcpy(&bits32, &f, sizeof(bits32));
                symbolTable.setGlobalInitializer(storageName, static_cast<long>(bits32));
            } else {
                symbolTable.setGlobalInitializer(storageName,
                        doubleBitsAsLong(static_cast<double>(initValue)));
            }
        } else {
            diagnostics.error("global floating initializer is not a constant expression",
                    declarator.getContext());
        }
        return;
    }
    if (effectiveInit->evaluateConstant(initValue)) {
        symbolTable.setGlobalInitializer(storageName, initValue);
        return;
    }
    std::string dataOperand;
    if (tryFoldDataOperand(effectiveInit, type, store, dataOperand)) {
        symbolTable.setGlobalAddressInitializer(storageName, dataOperand);
        return;
    }
    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(initAst);
            list && type.isAggregate()) {
        type::Type objectType = type;
        type::Type completedType = type;
        std::vector<std::string> words;
        AggregateInitHost host {
            store,
            [this](std::string msg, const translation_unit::Context& ctx) {
                diagnostics.error(std::move(msg), ctx);
            }
        };
        if (lowerToDataWords(objectType, list, host, completedType, words)) {
            if (completedType.getSize() != type.getSize()) {
                symbolTable.setGlobalType(storageName, completedType);
                symbolTable.setLocalType(declarator.getName(), completedType);
                store.setValue(&declarator, symbols::ValueSlot::Holder, symbolTable.lookup(declarator.getName()));
                type = completedType;
            }
            symbolTable.setGlobalMultiWordInitializer(storageName, std::move(words));
        }
        return;
    }
    diagnostics.error("global initializer is not a constant expression", declarator.getContext());
}

void DeclarationAnalyzer::applyLocalAggregateInitializer(ast::InitializedDeclarator& declarator,
        ast::Expression* initAst) {
    type::Type holderType = store.value(&declarator, symbols::ValueSlot::Holder)->getType();
    // Incomplete-array completion lives only in lowerToFieldInits; update the symbol if completed.
    AggregateInitHost host {
        store,
        [this](std::string msg, const translation_unit::Context& ctx) {
            diagnostics.error(std::move(msg), ctx);
        }
    };
    type::Type completed = lowerToFieldInits(holderType, initAst, symbolTable, host,
            [&](symbols::StructFieldInit init) {
                store.addStructFieldInit(&declarator, std::move(init));
            });
    if (completed.getSize() != holderType.getSize()
            || (completed.isArray() && holderType.isArray()
                    && completed.getArraySize() != holderType.getArraySize())) {
        symbolTable.setLocalType(declarator.getName(), completed);
        store.setValue(&declarator, symbols::ValueSlot::Holder,
                symbolTable.lookup(declarator.getName()));
    }
}

} // namespace semantic_analyzer
