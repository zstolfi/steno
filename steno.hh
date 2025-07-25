#pragma once
#include <iostream>
#include <bitset>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <span>
#include <initializer_list>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr struct FromBits_Arg {} FromBits;
constexpr struct FromBitsReversed_Arg {} FromBitsReversed;

enum struct Key {
	Num = 0,
	S_, T_, K_, P_, W_, H_, R_,
	A, O, x, E, U,
	_F, _R, _P, _B, _L, _G, _T, _S, _D, _Z,
	Mark = 23, OpenLeft, OpenRight,
//	FailedConstruction = 31
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

union Stroke {
	std::bitset<32> bits {};
	struct {
		/* Number Bar: */
		unsigned Num:1;
		/* Initial Consonants: */
		unsigned S_:1, T_:1, K_:1, P_:1, W_:1, H_:1, R_:1;
		/* Vowels: */
		unsigned A:1, O:1, x:1, E:1, U:1;
		/* Final Consonants: */
		unsigned _F:1, _R:1, _P:1, _B:1, _L:1, _G:1, _T:1, _S:1, _D:1, _Z:1;
		/* Flags: */
		unsigned Mark:1, OpenLeft:1, OpenRight:1;
		unsigned /*padding*/:5, FailedConstruction:1;
	} keys;

public:
	Stroke() = default;
	Stroke(std::string_view);
	Stroke(FromBits_Arg, std::bitset<23>);
	Stroke(FromBitsReversed_Arg, std::bitset<23>);

	bool operator==(const Stroke other) const {
		return this->bits
		==     other.bits;
	}
	auto operator<=>(const Stroke other) const {
		return this->bits.to_ulong()
		<=>    other.bits.to_ulong();
	}

	bool failed() const;

	bool get(Key) const;
	Stroke set(Key);
	Stroke unset(Key);
	Stroke operator+=(Stroke);
	Stroke operator-=(Stroke);
	Stroke operator&=(Stroke);

private:
	void failConstruction(std::string_view = "");
};

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
