#include "util/Diagnostic.h"

#include <stdexcept>

namespace diag {

namespace {

const char* severityText(Severity severity) {
    switch (severity) {
    case Severity::Error:
        return "error";
    case Severity::Warning:
        return "warning";
    }
    throw std::logic_error { "unknown Severity" };
}

void writeOne(std::ostream& out, const Diagnostic& diagnostic) {
    out << diagnostic.where << ": " << severityText(diagnostic.severity) << ": "
            << diagnostic.message << "\n";
}

} // namespace

Sink::Sink(std::ostream& out) :
        out_ { out } {
}

void Sink::error(const translation_unit::Context& where, std::string message) {
    diags_.push_back({ Severity::Error, where, std::move(message) });
    writeOne(out_, diags_.back());
}

void Sink::warn(const translation_unit::Context& where, std::string message) {
    diags_.push_back({ Severity::Warning, where, std::move(message) });
    writeOne(out_, diags_.back());
}

bool Sink::hasErrors() const {
    for (const auto& d : diags_) {
        if (d.severity == Severity::Error) {
            return true;
        }
    }
    return false;
}

const std::vector<Diagnostic>& Sink::all() const {
    return diags_;
}

void Sink::formatTo(std::ostream& out) const {
    for (const auto& d : diags_) {
        writeOne(out, d);
    }
}

} // namespace diag
