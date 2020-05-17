#ifndef FUNCTIONENTRY_H_
#define FUNCTIONENTRY_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "types/FunctionType.h"
#include "translation_unit/Context.h"

namespace semantic_analyzer {

class FunctionEntry {
public:
    FunctionEntry(std::string name, ast::FunctionType type, translation_unit::Context context);
    virtual ~FunctionEntry();

    std::string getName() const;
    ast::FunctionType getType() const;
    translation_unit::Context getContext() const;

    std::size_t argumentCount() const;
    const std::vector<std::unique_ptr<ast::FundamentalType>>& arguments() const;
    const ast::FundamentalType& returnType() const;

private:
    std::string name;
    ast::FunctionType type;
    translation_unit::Context context;
};

} /* namespace semantic_analyzer */

#endif /* FUNCTIONENTRY_H_ */
