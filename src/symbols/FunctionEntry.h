#ifndef SYMBOLS_FUNCTIONENTRY_H_
#define SYMBOLS_FUNCTIONENTRY_H_

#include <memory>
#include <string>
#include <vector>

#include "types/Function.h"
#include "translation_unit/Context.h"

namespace symbols {

class FunctionEntry {
public:
    FunctionEntry(std::string name, type::Function type, translation_unit::Context context);
    virtual ~FunctionEntry();

    std::string getName() const;
    type::Function getType() const;
    translation_unit::Context getContext() const;

    std::size_t argumentCount() const;
    std::vector<type::Type> arguments() const;
    type::Type returnType() const;

private:
    std::string name;
    type::Function type;
    translation_unit::Context context;
};

} // namespace symbols

#endif // SYMBOLS_FUNCTIONENTRY_H_
