#ifndef AGGREGATE_INIT_SINKS_H_
#define AGGREGATE_INIT_SINKS_H_

#include "AggregateInitSink.h"
#include "SymbolTable.h"

#include "symbols/AnnotationTypes.h"
#include "symbols/GlobalInitializer.h"
#include "translation_unit/Context.h"

#include <string>
#include <vector>

namespace semantic_analyzer {

struct FieldPlanSink : AggregateInitSink {
    AggregateInitHost& host;
    SymbolTable& symbolTable;
    translation_unit::Context context;
    std::vector<symbols::FieldInit>& plan;
    bool failed { false };

    FieldPlanSink(AggregateInitHost& host, SymbolTable& st, translation_unit::Context ctx,
            std::vector<symbols::FieldInit>& p);

    bool ok() const override;
    void error(const std::string& message) override;
    void onUnwritten(int offsetBytes, const type::Type& t) override;
    void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value,
            std::optional<type::BitField> bits = {}) override;
    bool placeStringArray(int offsetBytes, const type::Type& arrayType,
            ast::Expression* value) override;
    bool placeAggregateCopy(int offsetBytes, const type::Type& storeType,
            ast::Expression* value) override;
};

struct DataWordSink : AggregateInitSink {
    AggregateInitHost& host;
    translation_unit::Context context;
    std::vector<symbols::DataWord>& words;
    int wordCount;
    bool failed { false };

    DataWordSink(AggregateInitHost& host, translation_unit::Context ctx,
            std::vector<symbols::DataWord>& w, int wc);

    bool ok() const override;
    void error(const std::string& message) override;
    void onUnwritten(int offsetBytes, const type::Type& t) override;
    void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value,
            std::optional<type::BitField> bits = {}) override;
    bool placeStringArray(int offsetBytes, const type::Type& arrayType,
            ast::Expression* value) override;
};

} // namespace semantic_analyzer

#endif
