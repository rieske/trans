#ifndef TRANSLATIONUNITCONTEXT_H_
#define TRANSLATIONUNITCONTEXT_H_

#include <cstddef>
#include <string>

class TranslationUnitContext {
public:
	TranslationUnitContext(const std::string& sourceName, std::size_t offset);
	virtual ~TranslationUnitContext();

	size_t getOffset() const;
	const std::string& getSourceName() const;

private:
	std::string sourceName;
	std::size_t offset;
};

std::ostream& operator<<(std::ostream& ostream, const TranslationUnitContext& context);

#endif /* TRANSLATIONUNITCONTEXT_H_ */
