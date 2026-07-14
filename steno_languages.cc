#include "steno_languages.hh"

namespace /*details*/ {

std::string combineEnglish(std::string_view lhs, std::string_view rhs) {
#if 0
	/* Comples rules go here ... */
#else
	return lhs + rhs;
#endif
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
		return lhs + rhs;
	}
}

}
