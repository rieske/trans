#ifndef LANGUAGE_FRONT_END_H_
#define LANGUAGE_FRONT_END_H_

#include <memory>
#include <string>

#include "Configuration.h"
#include "parser/Grammar.h"
#include "parser/ParsingTable.h"

class LanguageFrontEnd {
public:
    LanguageFrontEnd(const LanguageFrontEnd&) = delete;
    LanguageFrontEnd& operator=(const LanguageFrontEnd&) = delete;
    LanguageFrontEnd(LanguageFrontEnd&&) = delete;
    LanguageFrontEnd& operator=(LanguageFrontEnd&&) = delete;

    static std::shared_ptr<const LanguageFrontEnd> load(const Configuration& configuration);
    static void clearProductCacheForTesting();

    const parser::Grammar& grammar() const;
    const parser::ParsingTable& table() const;

private:
    explicit LanguageFrontEnd(parser::Grammar grammar);

    static std::shared_ptr<const LanguageFrontEnd> product(const Configuration& configuration);
    static std::shared_ptr<const LanguageFrontEnd> generate(const Configuration& configuration);
    static std::shared_ptr<LanguageFrontEnd> withFileTable(
            parser::Grammar grammar, const std::string& tablePath);
    static std::shared_ptr<LanguageFrontEnd> withGeneratedTable(parser::Grammar grammar);

    parser::Grammar grammar_;
    std::unique_ptr<parser::ParsingTable> table_;
};

#endif
