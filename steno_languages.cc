#include "steno.hh"
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
	// Simple punctuation
	if (str == ",") return {English};
	if (str == ".") return {English};
	if (str == "?") return {English};
	if (str == "!") return {English};
	if (str == ";") return {English};
	if (str == ":") return {English};
	// Capitalization
	if (str == "-|") return {English};

	return {};
}

void accommodateWord(std::ostream& os, Context& c, Word word) {
	using enum State::Position;
	if (c.language() == English) {
		if (c.state().position == WordStart) os << " ";
		if (c.state().position == SentenceStart) os << " ";
		os << word;
	}
}

void processPunctuation(std::ostream& os, Context& c, std::string_view symbol) {
	using enum State::Position;
	if (c.language() == English) {
		/**/ if (symbol == ",") os << ",", c.state().position = WordStart;
		else if (symbol == ".") os << ".", c.state().position = SentenceStart;
		else if (symbol == "?") os << "?", c.state().position = SentenceStart;
		else if (symbol == "!") os << "!", c.state().position = SentenceStart;
		else if (symbol == ";") os << ";", c.state().position = WordStart;
		else if (symbol == ":") os << ":", c.state().position = WordStart;
//		else if (symbol == "-|") c.state(English)->capitalize = true;
	}
}

}
