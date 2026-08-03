#ifndef SYMBOLS_LABELENTRY_H_
#define SYMBOLS_LABELENTRY_H_

#include <string>

namespace symbols {

class LabelEntry {
public:
    LabelEntry(std::string name);

    std::string getName() const;

private:
    std::string name;
};

} // namespace symbols

#endif // SYMBOLS_LABELENTRY_H_
