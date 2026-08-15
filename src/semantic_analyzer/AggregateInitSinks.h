#ifndef AGGREGATE_INIT_SINKS_H_
#define AGGREGATE_INIT_SINKS_H_

#include "AggregateInitSink.h"
#include "SemanticAnalysisVisitor.h"
#include "SymbolTable.h"

#include "symbols/AnnotationStore.h"
#include "symbols/StaticInit.h"
#include "translation_unit/Context.h"

#include <string>
#include <vector>

namespace semantic_analyzer {

struct FieldPlanSink : AggregateInitSink {
    SemanticAnalysisVisitor& visitor;
    SymbolTable& symbolTable;
    symbols::AnnotationStore& annotations;
    translation_unit::Context context;
    std::vector<symbols::StructFieldInit>& plan;
    bool failed { false };

    FieldPlanSink(SemanticAnalysisVisitor& v, SymbolTable& st, symbols::AnnotationStore& ann,
            translation_unit::Context ctx, std::vector<symbols::StructFieldInit>& p);

    bool ok() const override;
    void error(const std::string& message) override;
    void onUnwritten(const type::FoundMember& slot) override;
    void placeScalar(const type::FoundMember& slot, ast::Expression* value) override;
    void placeInteger(const type::FoundMember& slot, long value) override;
};

struct DataWordSink : AggregateInitSink {
    SemanticAnalysisVisitor& visitor;
    translation_unit::Context context;
    std::vector<symbols::StaticInitValue>& words;
    int wordCount;
    bool failed { false };

    DataWordSink(SemanticAnalysisVisitor& v, translation_unit::Context ctx,
            std::vector<symbols::StaticInitValue>& w, int wc);

    bool ok() const override;
    void error(const std::string& message) override;
    void onUnwritten(const type::FoundMember& slot) override;
    void placeScalar(const type::FoundMember& slot, ast::Expression* value) override;
    void placeInteger(const type::FoundMember& slot, long value) override;
};

} // namespace semantic_analyzer

#endif
