#ifndef TRANSLATIONUNITCONTEXTAWARE_H_
#define TRANSLATIONUNITCONTEXTAWARE_H_

#include "translation_unit/Context.h"

namespace ast {

class TranslationUnitContextAware {
public:
    translation_unit::Context getContext() const;

protected:
    TranslationUnitContextAware(translation_unit::Context context);

private:
    translation_unit::Context context;
};

} /* namespace ast */

#endif /* TRANSLATIONUNITCONTEXTAWARE_H_ */
