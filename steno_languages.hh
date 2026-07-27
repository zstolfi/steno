#pragma once
#include <array>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace steno {

/* ~~ English ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct English_Arg {
	static constexpr std::string_view Name {"English"};
	static constexpr LanguageCode Code {"eng", "Latn"};

	// Standard punctuation
	static constexpr Signal Comma            {Punctuate, ","};
	static constexpr Signal Period           {Punctuate, "."};
	static constexpr Signal QuestionMark     {Punctuate, "?"};
	static constexpr Signal ExclamationPoint {Punctuate, "!"};
	static constexpr Signal Semicolon        {Punctuate, ";"};
	static constexpr Signal Colon            {Punctuate, ":"};
	// Invisible punctuation
	static constexpr Signal Combine          {Punctuate, "^"};
	static constexpr Signal DigitSequence    {Punctuate, "&"};
	static constexpr Signal Capitalize       {Punctuate, "-|"};

	static constexpr std::array Punctuation {
		Comma, Period, QuestionMark, ExclamationPoint, Semicolon, Colon,
		Combine, DigitSequence, Capitalize,
	};

	struct State {
		static constexpr auto Language() { return English_Arg {}; };

		enum Position {
			WordMiddle,
			WordStart,
			ProperStart,
			SentenceStart,
			ParagraphStart,
			DigitSequence,
		} position {WordStart};

		bool forceCapitalize {false};

		State(Default_Arg={}) {}
		State(Opening_Arg): position{ParagraphStart} {}

		bool operator== (State const&) const = default;
		auto operator<=>(State const&) const = default;

		// TODO: Put these in Language_Arg concept.
		static void applyWord(std::ostream&, Word);
		static void applyPunctuation(std::ostream&, std::string_view);
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

struct Languages : ValueList<NoLanguage, English/*, ... */> {
private:
	template <Language_Arg L> using GetState = L::State;

public:
	using States = Types::Map<GetState>;
};

// TODO: Make this value computed at compile time.
static std::set const GlobalPunctuation {
	English.Comma,
	English.Period,
	English.QuestionMark,
	English.ExclamationPoint,
	English.Semicolon,
	English.Colon,
	English.Combine,
	English.DigitSequence,
	English.Capitalize,
};

/* ~~ Orthography ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Orthographies define the most common pattern in a language's spelling.
// As an English example: take + -ing = taking, and tap + -ing = tapping.
// We know to remove the 'e' and double the 'p' solely because we are writing in
// the English language, and normal rules apply. Irregular rules are handled by
// user-defined dictionaries.

} // namespace steno
