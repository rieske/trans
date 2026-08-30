#include "LanguageFrontEnd.h"

#include "parser/BNFFileReader.h"

#include <mutex>
#include <utility>

namespace {

std::mutex cacheMutex;
std::shared_ptr<const LanguageFrontEnd> cachedProduct;
std::string cachedGrammarPath;

parser::Grammar readGrammar(const Configuration& configuration) {
    parser::BNFFileReader reader;
    return reader.readGrammar(configuration.getGrammarPath());
}

} // namespace

LanguageFrontEnd::LanguageFrontEnd(parser::Grammar grammar) :
        grammar_ { std::move(grammar) },
        table_ { &grammar_ } {
}

const parser::Grammar& LanguageFrontEnd::grammar() const {
    return grammar_;
}

const parser::ParsingTable& LanguageFrontEnd::table() const {
    return table_;
}

std::shared_ptr<const LanguageFrontEnd> LanguageFrontEnd::load(const Configuration& configuration) {
    const std::string grammarPath = configuration.getGrammarPath();

    std::lock_guard<std::mutex> lock { cacheMutex };
    if (cachedProduct && cachedGrammarPath == grammarPath) {
        return cachedProduct;
    }

    cachedProduct.reset(new LanguageFrontEnd { readGrammar(configuration) });
    cachedGrammarPath = grammarPath;
    return cachedProduct;
}

void LanguageFrontEnd::clearProductCacheForTesting() {
    std::lock_guard<std::mutex> lock { cacheMutex };
    cachedProduct.reset();
    cachedGrammarPath.clear();
}
