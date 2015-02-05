#include "TranslationUnitContextAware.h"

namespace ast {

TranslationUnitContextAware::TranslationUnitContextAware(translation_unit::Context) :
        context { context }
{
}

translation_unit::Context TranslationUnitContextAware::getContext() const {
    return context;
}

} /* namespace ast */
