#ifndef LANGUAGE_FRONT_END_H_
#define LANGUAGE_FRONT_END_H_

#include <memory>
#include <string>

#include "Configuration.h"
#include "parser/Grammar.h"
#include "parser/ParsingTable.h"

class LanguageFrontEnd {
    struct CtorTag {
        explicit CtorTag() = default;
    };

public:
    LanguageFrontEnd(const LanguageFrontEnd&) = delete;
    LanguageFrontEnd& operator=(const LanguageFrontEnd&) = delete;
    LanguageFrontEnd(LanguageFrontEnd&&) = delete;
    LanguageFrontEnd& operator=(LanguageFrontEnd&&) = delete;

    explicit LanguageFrontEnd(CtorTag, parser::Grammar grammar);

    static std::shared_ptr<const LanguageFrontEnd> load(const Configuration& configuration);
    static void clearProductCacheForTesting();

    const parser::Grammar& grammar() const;
    const parser::ParsingTable& table() const;

private:
    parser::Grammar grammar_;
    parser::ParsingTable table_;
};

#endif
