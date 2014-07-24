#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <sstream>

#include "semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "semantic_analyzer/ArrayAccess.h"

using namespace semantic_analyzer;

using testing::Eq;

namespace {

/*TEST(SemanticXmlOutputVisitor, outputsArrayAccess) {
	ArrayAccess node;

	std::ostringstream stream;
	SemanticXmlOutputVisitor visitor { &stream };

	node.accept(visitor);

	EXPECT_THAT(stream.str(), Eq("<array_access>\n"
			"</array_access>\n"));
}*/

}
