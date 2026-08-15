#ifndef SYMBOLS_LABELENTRY_H_
#define SYMBOLS_LABELENTRY_H_

#include <string>

namespace symbols {

class LabelEntry {
public:
    explicit LabelEntry(std::string name);

    const std::string& getName() const;

private:
    std::string name;
};

} // namespace symbols

#endif // SYMBOLS_LABELENTRY_H_
