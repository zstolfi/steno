#include "steno_languages.hh"

namespace /*details*/ {

std::string combineEnglish(std::string_view lhs, std::string_view rhs) {
	/* TODO */;
	return std::string {lhs} + std::string {rhs};
}

}; // namespace /*details*/

namespace steno {

std::string LanguageCode::name() const {
	if (m_name == NoLanguageCode.m_name) return "";
	return {m_name.begin(), m_name.end()};
}

std::string LanguageCode::script() const {
	if (m_script == NoLanguageCode.m_script) return "";
	return {m_script.begin(), m_script.end()};
}

std::string LanguageCode::region() const {
	if (m_region == NoLanguageCode.m_region) return "";
	return {m_region.begin(), m_region.end()};
}


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
