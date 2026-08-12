#include "LanguageFrontEnd.h"

#include "parser/BNFFileReader.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/GeneratedParsingTable.h"

#include <chrono>
#include <iostream>
#include <mutex>
#include <utility>

namespace {

std::mutex cacheMutex;
std::shared_ptr<const LanguageFrontEnd> cachedProduct;
std::string cachedGrammarPath;
std::string cachedTablePath;

parser::Grammar readGrammar(const Configuration& configuration) {
    parser::BNFFileReader reader;
    return reader.readGrammar(configuration.getGrammarPath());
}

std::unique_ptr<parser::ParsingTable> generateTable(parser::Grammar& grammar) {
    std::cout << "Generating parsing table" << std::endl;
    const auto begin = std::chrono::steady_clock::now();
    auto table = std::make_unique<parser::GeneratedParsingTable>(&grammar);
    const auto end = std::chrono::steady_clock::now();
    std::cout << "Parsing table generation took "
            << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
            << "[µs]" << std::endl;
    const std::string parsingTableLocation { "logs/parsing_table" };
    table->persistToFile(parsingTableLocation);
    std::cout << "Parsing table saved to: " << parsingTableLocation << std::endl;
    return table;
}

} // namespace

LanguageFrontEnd::LanguageFrontEnd(parser::Grammar grammar) :
        grammar_ { std::move(grammar) } {
}

std::shared_ptr<LanguageFrontEnd> LanguageFrontEnd::withFileTable(
        parser::Grammar grammar, const std::string& tablePath) {
    std::shared_ptr<LanguageFrontEnd> frontEnd { new LanguageFrontEnd { std::move(grammar) } };
    frontEnd->table_ = std::make_unique<parser::FilePersistedParsingTable>(tablePath, &frontEnd->grammar_);
    return frontEnd;
}

std::shared_ptr<LanguageFrontEnd> LanguageFrontEnd::withGeneratedTable(parser::Grammar grammar) {
    std::shared_ptr<LanguageFrontEnd> frontEnd { new LanguageFrontEnd { std::move(grammar) } };
    frontEnd->table_ = generateTable(frontEnd->grammar_);
    return frontEnd;
}

const parser::Grammar& LanguageFrontEnd::grammar() const {
    return grammar_;
}

const parser::ParsingTable& LanguageFrontEnd::table() const {
    return *table_;
}

std::shared_ptr<const LanguageFrontEnd> LanguageFrontEnd::load(const Configuration& configuration) {
    if (configuration.usingCustomGrammar()) {
        return generate(configuration);
    }
    return product(configuration);
}

std::shared_ptr<const LanguageFrontEnd> LanguageFrontEnd::product(const Configuration& configuration) {
    const std::string grammarPath = configuration.getGrammarPath();
    const std::string tablePath = configuration.getParsingTablePath();

    std::lock_guard<std::mutex> lock { cacheMutex };
    if (cachedProduct && cachedGrammarPath == grammarPath && cachedTablePath == tablePath) {
        return cachedProduct;
    }

    cachedProduct = withFileTable(readGrammar(configuration), tablePath);
    cachedGrammarPath = grammarPath;
    cachedTablePath = tablePath;
    return cachedProduct;
}

std::shared_ptr<const LanguageFrontEnd> LanguageFrontEnd::generate(const Configuration& configuration) {
    return withGeneratedTable(readGrammar(configuration));
}

void LanguageFrontEnd::clearProductCacheForTesting() {
    std::lock_guard<std::mutex> lock { cacheMutex };
    cachedProduct.reset();
    cachedGrammarPath.clear();
    cachedTablePath.clear();
}
