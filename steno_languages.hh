#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace steno {

/* ~~ English ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct English_Arg {
	static constexpr std::string_view Name {"English"};
	static constexpr LanguageCode Identifier {"eng", "Latn"};

	struct State {
		using Language_Arg = English_Arg;
		static constexpr auto Language() { return Language_Arg {}; };

		enum Position {
			WordMiddle,
			WordStart,
			ProperStart,
			SentenceStart,
			ParagraphStart,
			DigitSequence,
		} position;

		State(Opening_Arg): position{ParagraphStart} {}
		State(Default_Arg): position{WordStart} {}

		bool operator== (State const&) const = default;
		auto operator<=>(State const&) const = default;
	};
};
static constexpr auto English = English_Arg {};

/* ~~ Default Language ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef STENO_DEFAULT_LANGUAGE
	// English by default is opt-out.
	using DefaultLanguage_Arg = English_Arg;
	static constexpr auto DefaultLanguage = English;
#else
	using DefaultLanguage_Arg = STENO_DEFAULT_LANGUAGE##_Arg;
	static constexpr auto DefaultLanguage = STENO_DEFAULT_LANGUAGE;
#endif

/* ~~ Complete List ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Languages : ValueList<English, French> {
private:
	template <Language_Arg L> using GetState = L::State;

public:
	using States = Types::Map<GetState>;
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
