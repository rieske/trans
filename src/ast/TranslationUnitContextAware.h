#ifndef TRANSLATIONUNITCONTEXTAWARE_H_
#define TRANSLATIONUNITCONTEXTAWARE_H_

#include "../translation_unit/Context.h"

namespace ast {

class TranslationUnitContextAware {
public:
    TranslationUnitContextAware(translation_unit::Context context);
    virtual ~TranslationUnitContextAware() = default;

    translation_unit::Context getContext() const;

protected:
    TranslationUnitContextAware(const TranslationUnitContextAware&) = default;
    TranslationUnitContextAware(TranslationUnitContextAware&&) = default;
    TranslationUnitContextAware& operator=(const TranslationUnitContextAware&) = default;
    TranslationUnitContextAware& operator=(TranslationUnitContextAware&&) = default;

private:
    translation_unit::Context context;
};

} /* namespace ast */

#endif /* TRANSLATIONUNITCONTEXTAWARE_H_ */
