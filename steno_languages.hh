#pragma once
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <concepts>
#include <cassert>

namespace steno {

enum Language {
	NoLanguage,
	English,
};

static constexpr auto DefaultLanguage = Language {
#ifdef STENO_DEFAULT_LANGUAGE
	STENO_DEFAULT_LANGUAGE
#else
	English // English by default is opt-out.
#endif
};

/* ~~ Language Identifier ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Scripts are a mandatory part of language identification. This is okay
// because this library only handles the written word, and never any semantic
// meaning. Region, however, is only to be used when two cultures' use of a
// language differ so much that their combining rules differ. Most languages
// used across multiple regions have identical rules, so this is rarely needed.

class LanguageCode {
	std::array<char, 3> m_name {}; // ISO 639-2/T
	std::array<char, 4> m_script {}; // ISO 15924
	std::array<char, 2> m_region {}; // ISO 3166-1 alpha-2

	constexpr LanguageCode(
		std::string_view name,
		std::string_view script,
		std::string_view region={"AA"}
	) {
		for (int i=0; i<3; i++) m_name[i] = name[i];
		for (int i=0; i<4; i++) m_script[i] = script[i];
		for (int i=0; i<2; i++) m_region[i] = region[i];
	}

public:
	// By default we use reserved values to denote lack of code/script/region.
	constexpr LanguageCode(): LanguageCode{"qaa", "Qaaa", "AA"} {}
	constexpr LanguageCode(Language language) {
		switch (language) {
		case English:
			*this = LanguageCode {"eng", "Latn"}; break;
/*
		// Further examples:
		case EnglishBraille: // ⠠⠢⠛⠇⠊⠩⠀⠠⠃⠗⠇
			*this = LanguageCode {"eng", "Brai"}; break;

		case JapaneseBraille: // ⠇⠮⠴⠐⠪⠎⠀⠟⠴⠐⠳
			*this = LanguageCode {"jpn", "Brai"}; break;

		case Mongolian: // Монгол хэл
			*this = LanguageCode {"mon", "Cyrl"}; break;

		case MongolianTraditional: // ᠮᠣᠩᠭᠣᠯ ᠬᠡᠯᠡ
			*this = LanguageCode {"mon", "Mong"}; break;
*/
		default: assert(language == NoLanguage);
			*this = LanguageCode {};
		}
	}

	// Comparison
	bool operator== (LanguageCode const&) const = default;
	auto operator<=>(LanguageCode const&) const = default;

	// Getters
	std::string name  () const;
	std::string script() const;
	std::string region() const;
};

static constexpr auto NoLanguageCode = LanguageCode {};

/* ~~ Orthography ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Orthographies define the most common pattern in a language's spelling.
// As an English example: take + -ing = taking, and tap + -ing = tapping.
// We know to remove the 'e' and double the 'p' solely because we are writing in
// the English language, and normal rules apply. Irregular rules are handled by
// user-defined dictionaries.

std::string combine(Language, std::string_view, std::string_view);

std::vector<Language> recognizedPunctuation(std::string_view);

} // namespace steno
