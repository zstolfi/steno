#pragma once
#include <concepts>
#include <string_view>

namespace steno {

/* ~~ Locale Codes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Scripts will be a part of a language's definition. This is okay because
// this library only concerns writing, and not language semantics.

class LanguageCode {
	char value[3 + 4]; // ISO 639-2/T  +  ISO 15924

public:
	constexpr LanguageCode(std::string_view lang, std::string_view script) {
		assert(lang.size() == 3);
		assert('a' <= lang[0] && lang[0] <= 'z'), value[0] = lang[0];
		assert('a' <= lang[1] && lang[1] <= 'z'), value[1] = lang[1];
		assert('a' <= lang[2] && lang[2] <= 'z'), value[2] = lang[2];
		assert(script.size() == 4);
		assert('A' <= script[0] && script[0] <= 'Z'), value[3] = script[0];
		assert('a' <= script[1] && script[1] <= 'z'), value[4] = script[1];
		assert('a' <= script[2] && script[2] <= 'z'), value[5] = script[2];
		assert('a' <= script[3] && script[3] <= 'z'), value[6] = script[3];
	}

	bool operator== (LanguageCode const&) const = default;
	auto operator<=>(LanguageCode const&) const = default;

	// Getters
	operator std::string_view() const;
	std::array<std::string_view, 2> split() const;
};

//   Languages are not specific to any one region. However, they are allowed to
// have their orthographic rules depend on region. (For example, Portuguese
// pre-1990.)

class RegionCode {
	char value[2]; // ISO 3166-1 alpha-2

public:
	RegionCode(std::string_view str) {
		assert(str.size() == 2);
		assert ('A' <= str[0] && str[0] <= 'Z'), value[0] = str[0];
		assert ('A' <= str[1] && str[1] <= 'Z'), value[1] = str[1];
	}

	bool operator== (RegionCode const&) const = default;
	auto operator<=>(RegionCode const&) const = default;

	// Getters
	operator std::string_view() const;
};

/* ~~ Language Classes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <class T>
concept Language = requires(T) {
	{ T::Code } -> std::convertible_to<LanguageCode>;
};

struct NoLanguage {
	// We use reserved values to denote "no code" and "no script".
	static constexpr LanguageCode Code {"qaa", "Qaaa"};
};

struct English {
	static constexpr LanguageCode Code {"eng", "Latn"};
};

// Further examples:

/*
struct EnglishBraille {
	// ⠠⠢⠛⠇⠊⠩⠀⠠⠃⠗⠇
	static constexpr std::string_view Code {"eng", "Brai"};
};

struct JapaneseBraille {
	// ⠇⠮⠴⠐⠪⠎⠀⠟⠴⠐⠳
	static constexpr std::string_view Code {"jpn", "Brai"};
};

struct Mongolian {
	// Монгол хэл
	static constexpr std::string_view Code {"mon", "Cyrl"};
};

struct MongolianTraditional {
	// ᠮᠣᠩᠭᠣᠯ ᠬᠡᠯᠡ
	static constexpr std::string_view Code {"mon", "Mong"};
};
*/

} // namespace steno
