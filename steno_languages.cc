#include "steno_languages.hh"

namespace steno {

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
	// ⠠⠢⠛⠇⠊⠩⠀⠠⠃⠗⠇
	static constexpr LanguageCode Code {"eng", "Brai"};
};

struct Orthography<JapaneseBraille> {
	// ⠇⠮⠴⠐⠪⠎⠀⠟⠴⠐⠳
	static constexpr LanguageCode Code {"jpn", "Brai"};
};

struct Orthography<Mongolian> {
	// Монгол хэл
	static constexpr LanguageCode Code {"mon", "Cyrl"};
};

struct Orthography<MongolianTraditional> {
	// ᠮᠣᠩᠭᠣᠯ ᠬᠡᠯᠡ
	static constexpr LanguageCode Code {"mon", "Mong"};
};
*/


}
