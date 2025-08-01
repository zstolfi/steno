#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <bitset>
#include <vector>
#include <map>
#include <span>
#include <initializer_list>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr struct FromBits_Arg {} FromBits;
constexpr struct FromBitsReversed_Arg {} FromBitsReversed;

enum struct Key {
	// Number Bar
	Num = 0,
	// Initial Consonants
	S_, T_, K_, P_, W_, H_, R_,
	// Vowels
	A, O, x, E, U,
	// Final Consonants
	_F, _R, _P, _B, _L, _G, _T, _S, _D, _Z,
	// Flags
	Mark = 23, OpenLeft, OpenRight,
//	FailedConstruction = 31
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Stroke {
	//                          keys        flags  fail-bit
	//                ┌──────────┴──────────┐┌┴┐     │
	//                #STKPWHRAO*EUFRPBLGTSDZ!~~     X
	uint32_t bits = 0b00000000000000000000000000000000;

public:
	Stroke() = default;
	Stroke(std::string_view);
	Stroke(FromBits_Arg, std::bitset<23>);
	Stroke(FromBitsReversed_Arg, std::bitset<23>);

	bool operator==(Stroke const&) const = default;
	auto operator<=>(Stroke const&) const = default;

	bool failed() const;
	operator bool() const;

	bool get(Key) const;
	Stroke set(Key, bool = true);
	Stroke unset(Key);

	Stroke operator+=(Stroke);
	Stroke operator-=(Stroke);
	Stroke operator&=(Stroke);

private:
	void failConstruction(std::string_view = "");
};

namespace KeyUnit {
	static steno::Stroke const
		Num {"#"},
		S_ {"S-"},
		T_ {"T-"}, K_ {"K-"},
		P_ {"P-"}, W_ {"W-"},
		H_ {"H-"}, R_ {"R-"},
		A  {"A"} , O  {"O"} ,
		x  {"*"},
		E  {"E"} , U  {"U"} ,
		_F {"-F"}, _R {"-R"},
		_P {"-P"}, _B {"-B"},
		_L {"-L"}, _G {"-G"},
		_T {"-T"}, _S {"-S"},
		_D {"-D"}, _Z {"-Z"}
	;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Strokes {
	std::vector<Stroke> list {};

public:
	Strokes() = default;
	Strokes(Stroke);
	Strokes(std::span<const Stroke>);
	Strokes(std::initializer_list<Stroke>);
	Strokes(std::string_view);

	bool operator== (const Strokes xx) const { return list ==  xx.list; }
	auto operator<=>(const Strokes xx) const { return list <=> xx.list; }

	bool failed() const;

	Stroke& operator[](std::size_t);
	Stroke  operator[](std::size_t) const;

	Strokes& append (Stroke );
	Strokes& prepend(Stroke );
	Strokes& append (Strokes);
	Strokes& prepend(Strokes);
	Strokes& operator|=(Strokes);

private:
	using List_t = decltype(list);
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Brief {
	Strokes strokes {};
	std::string text {};

public:
	Brief() = default;
	Brief(std::string_view, Strokes);
	Brief(std::string_view, Brief);

	bool operator==(const Brief& b) const = default;

	bool failed() const;

	Brief& operator+=(Brief);
	Brief& operator|=(Brief);

private:
	void appendText(std::string);
	void normalize();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

using Dictionary = std::map<steno::Strokes, std::string>;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Multiple keys at the same time:
Stroke  operator+(Stroke , Stroke );
Strokes operator+(Strokes, Stroke );
Strokes operator+(Stroke , Strokes);
Brief   operator+(Brief  , Brief  );
Brief   operator+(Strokes, Brief  );
Brief   operator+(Brief  , Strokes);
Brief   operator+(std::string, Brief);
Brief   operator+(Brief, std::string);

// Removing keys:
Stroke  operator-(Stroke , Stroke );
Strokes operator-(Strokes, Stroke );

// Multiple strokes in order:
Strokes operator|(Stroke , Stroke );
Strokes operator|(Strokes, Stroke );
Strokes operator|(Stroke , Strokes);
Strokes operator|(Strokes, Strokes);
Brief   operator|(Brief  , Brief  );
Brief   operator|(Strokes, Brief  );
Brief   operator|(Brief  , Strokes);
// Subset of keys:
Stroke  operator&(Stroke , Stroke );


const auto NoStroke = Stroke {};
const auto NoStrokes = Strokes {};
const auto NoBrief = Brief {};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string toString(Key);
std::string toString(Stroke);
std::string toString(Strokes);
std::ostream& operator<<(std::ostream&, Stroke);
std::ostream& operator<<(std::ostream&, Strokes);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
