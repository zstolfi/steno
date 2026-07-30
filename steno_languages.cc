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

Signal English_Arg::Comma() {
	return Signal {Punctuate, std::string {Symbols::Comma}};
}

Signal English_Arg::Period() {
	return Signal {Punctuate, std::string {Symbols::Period}};
}

Signal English_Arg::QuestionMark() {
	return Signal {Punctuate, std::string {Symbols::QuestionMark}};
}

Signal English_Arg::ExclamationPoint() {
	return Signal {Punctuate, std::string {Symbols::ExclamationPoint}};
}

Signal English_Arg::Semicolon() {
	return Signal {Punctuate, std::string {Symbols::Semicolon}};
}

Signal English_Arg::Colon() {
	return Signal {Punctuate, std::string {Symbols::Colon}};
}

Signal English_Arg::Combine() {
	return Signal {Punctuate, std::string {Symbols::Combine}};
}

Signal English_Arg::DigitSequence() {
	return Signal {Punctuate, std::string {Symbols::DigitSequence}};
}

Signal English_Arg::DigitSequenceEnd() {
	return Signal {Punctuate, std::string {Symbols::DigitSequenceEnd}};
}

Signal English_Arg::Capitalize() {
	return Signal {Punctuate, std::string {Symbols::Capitalize}};
}

} // namespace steno
