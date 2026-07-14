#pragma once
#include <concepts>
#include <string_view>

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
	char m_name[3]; // ISO 639-2/T
	char m_script[4]; // ISO 15924
	char m_region[2]; // ISO 3166-1 alpha-2

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
	// Constructor
	constexpr LanguageCode(Language language=NoLanguage) {
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
			// We use reserved values to denote unspecified code/script/region.
			*this = LanguageCode {"qaa", "Qaaa", "AA"}; break;
		}
	}

	// Comparison
	bool operator== (LanguageCode const&) const = default;
	auto operator<=>(LanguageCode const&) const = default;

	// Getters
	std::string name  () const { return m_name;   }
	std::string script() const { return m_script; }
	std::string region() const { return m_region; }
};

static constexpr auto NoLanguageCode = LanguageCode {};

/* ~~ Orthography ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Orthographies define the most common pattern in a language's spelling.
// As an English example: take + -ing = taking, and tap + -ing = tapping.
// We know to remove the 'e' and double the 'p' solely because we are writing in
// the English language, and normal rules apply. Irregular rules are handled by
// user-defined dictionaries.

std::string combine(Language, std::string_view, std::string_view);

} // namespace steno
