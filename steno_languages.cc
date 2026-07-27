#include "steno.hh"
#include "steno_languages.hh"

namespace steno {

void English_Arg::State::applyWord(std::ostream& os, Word word) {
	if (position == WordStart) os << " ";
	if (position == SentenceStart) os << " ";
	if (forceCapitalize) /* TODO */;
	os << std::string_view {word};
}

void English_Arg::State::applyPunctuation(
	std::ostream& os,
	std::string_view symbol
) {
	// Standard punctuation
	/**/ if (symbol == ",") os << ",", position = WordStart;
	else if (symbol == ".") os << ".", position = SentenceStart;
	else if (symbol == "?") os << "?", position = SentenceStart;
	else if (symbol == "!") os << "!", position = SentenceStart;
	else if (symbol == ";") os << ";", position = WordStart;
	else if (symbol == ":") os << ":", position = WordStart;
	// Invisible punctuation
	else if (symbol == "^") position = WordStart;
	else if (symbol == "&") position = WordMiddle;
	else if (symbol == "-|") forceCapitalize = true;
}

} // namespace steno
