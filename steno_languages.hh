#pragma once
#include <concepts>
#include <string_view>

namespace steno {

//   Scripts will be considered as sub-languages. This is okay because this
// library only concerns writing, and not language semantics.

template <class T>
concept Language = requires(T t) {
	// Codes are ISO 639-2/T values.
	{ T::Code } -> std::convertible_to<std::string_view>;
	requires T::Code.size() == 3;
    requires 'a' <= T::Code[0] && T::Code[0] <= 'z';
    requires 'a' <= T::Code[1] && T::Code[1] <= 'z';
    requires 'a' <= T::Code[2] && T::Code[2] <= 'z';

	// Scripts are ISO 15924 values.
	{ T::Script } -> std::convertible_to<std::string_view>;
	requires T::Script.size() == 4;
    requires 'A' <= T::Script[0] && T::Script[0] <= 'Z';
    requires 'a' <= T::Script[1] && T::Script[1] <= 'z';
    requires 'a' <= T::Script[2] && T::Script[2] <= 'z';
    requires 'a' <= T::Script[3] && T::Script[3] <= 'z';
};

struct NoLanguage {
	// We use reserved values to denote "no code" and "no script".
	static constexpr std::string_view Code {"qaa"};
	static constexpr std::string_view Script {"Qaaa"};
};

struct English {
	static constexpr std::string_view Code {"eng"};
	static constexpr std::string_view Script {"Latn"};
};

// Further examples:

/*
struct EnglishBraille {
	// ⠠⠢⠛⠇⠊⠩⠀⠠⠃⠗⠇
	static constexpr std::string_view Code {"eng"};
	static constexpr std::string_view Script {"Brai"};
};

struct JapaneseBraille {
	// ⠇⠮⠴⠐⠪⠎⠀⠟⠴⠐⠳
	static constexpr std::string_view Code {"jpn"};
	static constexpr std::string_view Script {"Brai"};
};

struct Mongolian {
	// Монгол хэл
	static constexpr std::string_view Code {"mon"};
	static constexpr std::string_view Script {"Cyrl"};
};

struct MongolianTraditional {
	// ᠮᠣᠩᠭᠣᠯ ᠬᠡᠯᠡ
	static constexpr std::string_view Code {"mon"};
	static constexpr std::string_view Script {"Mong"};
};
*/

} // namespace steno
