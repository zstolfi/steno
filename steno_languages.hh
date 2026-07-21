#pragma once
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <concepts>
#include <cassert>

namespace steno {

/* ~~ Language State ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <>
struct Context::State<English> {
	enum Position {
		WordMiddle,
		WordStart,
		SentenceStart,
		ParagraphStart,
		DigitSequence,
	} position {WordStart};

	bool capitalize {false};
};

/* ~~ Orthography ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Orthographies define the most common pattern in a language's spelling.
// As an English example: take + -ing = taking, and tap + -ing = tapping.
// We know to remove the 'e' and double the 'p' solely because we are writing in
// the English language, and normal rules apply. Irregular rules are handled by
// user-defined dictionaries.

// TODO: Put these in a namespace.

std::string combine(Language, std::string_view, std::string_view);

std::vector<Language> recognizedPunctuation(std::string_view);

void accommodateWord(std::ostream&, Context&, Word);

void processPunctuation(std::ostream&, Context&, std::string_view);

} // namespace steno
