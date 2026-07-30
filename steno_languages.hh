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

	struct Symbols {
		static constexpr std::string_view
			// Standard punctuation
			Comma {","},
			Period {"."},
			QuestionMark {"?"},
			ExclamationPoint {"!"},
			Semicolon {";"},
			Colon {":"},
			// Invisible punctuation
			Combine {"^"},
			DigitSequence {"&"},
			DigitSequenceEnd {"!&"},
			Capitalize {"-|"}
		;

		static constexpr std::array All {
			Comma, Period, QuestionMark, ExclamationPoint, Semicolon, Colon,
			Combine, DigitSequence, DigitSequenceEnd, Capitalize,
		};
	};

	static constexpr char Uppercase(char c) {
		if ('a' <= c&&c <= 'z') return c + ('A' - 'a');
		else return c;
	}

	static constexpr char Lowercase(char c) {
		if ('A' <= c&&c <= 'Z') return c + ('a' - 'A');
		else return c;
	}

	struct State {
		static constexpr auto Language() { return English_Arg {}; };

		enum Position {
			WordHead, WordTail,
			NumberHead, NumberTail, NumberEnd,
		} position {WordHead};

		bool beginning {false};
		bool capitalize {false};

		State(Default_Arg={}) {}
		State(Opening_Arg): beginning{true}, capitalize{true} {}

		bool operator== (State const&) const = default;
		auto operator<=>(State const&) const = default;

		void applyWord(std::ostream&, Word);
		void applyPunctuation(std::ostream&, std::string_view);
	};

	// API access to punctuation Signals
	// Example:
	// speech << "well" << English.Comma() << "I told you" << English.Period();
	static Signal Comma();
	static Signal Period();
	static Signal QuestionMark();
	static Signal ExclamationPoint();
	static Signal Semicolon();
	static Signal Colon();
	static Signal Combine();
	static Signal DigitSequence();
	static Signal DigitSequenceEnd();
	static Signal Capitalize();
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
	English_Arg::Symbols::Comma,
	English_Arg::Symbols::Period,
	English_Arg::Symbols::QuestionMark,
	English_Arg::Symbols::ExclamationPoint,
	English_Arg::Symbols::Semicolon,
	English_Arg::Symbols::Colon,
	English_Arg::Symbols::Combine,
	English_Arg::Symbols::DigitSequence,
	English_Arg::Symbols::DigitSequenceEnd,
	English_Arg::Symbols::Capitalize,
};

/* ~~ Orthography ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Orthographies define the most common pattern in a language's spelling.
// As an English example: take + -ing = taking, and tap + -ing = tapping.
// We know to remove the 'e' and double the 'p' solely because we are writing in
// the English language, and normal rules apply. Irregular rules are handled by
// user-defined dictionaries.

} // namespace steno
