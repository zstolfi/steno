#include "steno.hh"
#include "steno_languages.hh"

namespace steno {

void English_Arg::State::applyWord(std::ostream& os, Word word) {
	bool const withSpace {position == WordStart && !beginning};

	if (withSpace) os << " ";
	if (capitalize) word[0] = English.Uppercase(word[0]);
	os << std::string_view {word};

	beginning = false;
}

void English_Arg::State::applyPunctuation(
	std::ostream& os,
	std::string_view symbol
) {
	// Standard punctuation
	/**/ if (symbol == ",") os << ",", position = WordStart;
	else if (symbol == ".") os << ".", position = WordStart, capitalize = true;
	else if (symbol == "?") os << "?", position = WordStart, capitalize = true;
	else if (symbol == "!") os << "!", position = WordStart, capitalize = true;
	else if (symbol == ";") os << ";", position = WordStart;
	else if (symbol == ":") os << ":", position = WordStart;
	// Invisible punctuation
	else if (symbol == "^") position = WordMiddle;
	else if (symbol == "&") position = DigitSequence;
	else if (symbol == "-|") capitalize = true;

	beginning = false;
}

} // namespace steno
