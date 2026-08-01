#include "steno.hh"
#include "steno_languages.hh"

namespace steno {

void English_Arg::State::applyWord(std::ostream& os, Word word) {
	if (position == NumberEnd) position = WordHead;

	bool const withSpace {
		!beginning && (position == WordHead || position == NumberHead)
	};

	if (withSpace) os << " ";
	if (capitalize) word[0] = English.Uppercase(word[0]);
	os << std::string_view {word};

	if (position == WordHead || position == WordTail) *this = {Default};
	beginning = false;
}

void English_Arg::State::applyPunctuation(
	std::ostream& os,
	std::string_view symbol
) {
	// Standard punctuation
	/**/ if (symbol == ",") os << ",", *this = {Default};
	else if (symbol == ".") os << ".", *this = {Default}, capitalize = true;
	else if (symbol == "?") os << "?", *this = {Default}, capitalize = true;
	else if (symbol == "!") os << "!", *this = {Default}, capitalize = true;
	else if (symbol == ";") os << ";", *this = {Default};
	else if (symbol == ":") os << ":", *this = {Default};
	// Invisible punctuation
	else if (symbol == "^") position = WordTail;
	else if (symbol == "&" && position == NumberEnd) position = NumberTail;
	else if (symbol == "&") position = NumberHead;
	else if (symbol == "!&") position = NumberEnd;
	else if (symbol == "-|") capitalize = true;
}

#define DEFINE_SHORTHAND(Language, Name)                                       \
Signal Language##_Arg::Name() {                                                \
    return Signal {Punctuate, std::string {Symbols::Name}};                    \
}

DEFINE_SHORTHAND(English, Comma);
DEFINE_SHORTHAND(English, Period);
DEFINE_SHORTHAND(English, QuestionMark);
DEFINE_SHORTHAND(English, ExclamationPoint);
DEFINE_SHORTHAND(English, Semicolon);
DEFINE_SHORTHAND(English, Colon);
DEFINE_SHORTHAND(English, Combine);
DEFINE_SHORTHAND(English, DigitSequence);
DEFINE_SHORTHAND(English, DigitSequenceEnd);
DEFINE_SHORTHAND(English, Capitalize);

#undef DEFINE_SHORTHAND

} // namespace steno
