#include "util/Diagnostic.h"

namespace diag {

namespace {

void writeOne(std::ostream& out, const Diagnostic& diagnostic) {
    out << diagnostic.where << ": error: " << diagnostic.message << "\n";
}

} // namespace

Sink::Sink(std::ostream& out) :
        out_ { out } {
}

void Sink::error(const translation_unit::Context& where, std::string message) {
    diags_.push_back({ where, std::move(message) });
    writeOne(out_, diags_.back());
}

bool Sink::hasErrors() const {
    return !diags_.empty();
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
