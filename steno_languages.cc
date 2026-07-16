#include "steno_languages.hh"

namespace /*details*/ {

std::string combineEnglish(std::string_view lhs, std::string_view rhs) {
	/* TODO */;
	return std::string {lhs} + std::string {rhs};
}

}; // namespace /*details*/

namespace steno {

std::string combine(
	Language language,
	std::string_view lhs,
	std::string_view rhs
) {
	switch (language) {
	case English:
		return combineEnglish(lhs, rhs);
	default: assert(language == NoLanguage);
		return std::string {lhs} + std::string {rhs};
	}
}

std::vector<Language> recognizedPunctuation(std::string_view str) {
	if (str == "," || str == "."
	||  str == "?" || str == "!"
	||  str == ";" || str == ":") return {English};

	return {};
}

}
