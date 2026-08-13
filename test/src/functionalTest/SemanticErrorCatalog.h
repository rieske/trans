#ifndef TEST_SEMANTIC_ERROR_CATALOG_H_
#define TEST_SEMANTIC_ERROR_CATALOG_H_

#include "TestFixtures.h"

#include <string>

// Catalog of compile-time rejection contracts. One row = one diagnostic pin.
struct SemanticErrorCase {
    const char *name;
    const char *source;
    const char *errorFragment;
};

class SemanticErrorCatalog : public testing::TestWithParam<SemanticErrorCase> {};

#endif
