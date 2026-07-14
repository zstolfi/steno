#include "steno_languages.hh"

namespace steno {

template <>
constexpr LanguageCode<English> = {"eng", "Latn"};

template <>
struct Orthography<English> {
	static constexpr LanguageCode Code {"eng", "Latn"};

	static std::string Combine(std::string_view lhs, std::string_view rhs) {
		std::string result {};
		result.reserve(lhs.size() + rhs.size());
		result = lhs + rhs;
		return result;
	}
}

// Further examples:

/*
struct Orthography<EnglishBraille> {
	EnglishBraille
	// ⠠⠢⠛⠇⠊⠩⠀⠠⠃⠗⠇
	{"eng", "Brai"}
};

struct Orthography<JapaneseBraille> {
	JapaneseBraille
	// ⠇⠮⠴⠐⠪⠎⠀⠟⠴⠐⠳
	{"jpn", "Brai"}
};

struct Orthography<Mongolian> {
	Mongolian
	// Монгол хэл
	{"mon", "Cyrl"}
};

struct Orthography<MongolianTraditional> {
	MongolianTraditional
	// ᠮᠣᠩᠭᠣᠯ ᠬᠡᠯᠡ
	{"mon", "Mong"}
};
*/


}
