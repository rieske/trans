#ifndef UTIL_DIAGNOSTIC_H_
#define UTIL_DIAGNOSTIC_H_

#include "translation_unit/Context.h"

#include <ostream>
#include <string>
#include <vector>

namespace diag {

enum class Severity { Error, Warning };

struct Diagnostic {
    Severity severity { Severity::Error };
    translation_unit::Context where { "", 0 };
    std::string message;
};

class Sink {
public:
    explicit Sink(std::ostream& out);
    void error(const translation_unit::Context& where, std::string message);
    void warn(const translation_unit::Context& where, std::string message);
    bool hasErrors() const;
    const std::vector<Diagnostic>& all() const;
    void formatTo(std::ostream& out) const;

private:
    std::ostream& out_;
    std::vector<Diagnostic> diags_;
};

} // namespace diag

#endif // UTIL_DIAGNOSTIC_H_
