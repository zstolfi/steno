#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include <forward_list>
#include <span>
#include <initializer_list>
#include <any>
#include <functional>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr struct FromBits_Arg {} FromBits;
constexpr struct FromBitsReversed_Arg {} FromBitsReversed;
constexpr struct Glue_Arg {} Glue;

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
	bool operator==(const Stroke other) const {
		return this->bits
		==     other.bits;
	}
	auto operator<=>(const Stroke other) const {
		return this->bits.to_ulong()
		<=>    other.bits.to_ulong();
	}


	Stroke(std::string);
	Stroke(FromBits_Arg, std::bitset<23>);
	Stroke(FromBitsReversed_Arg, std::bitset<23>);

	Stroke operator+=(Stroke);
	Stroke operator-=(Stroke);
	Stroke operator&=(Stroke);

private:
	void failConstruction();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Strokes {
	std::vector<Stroke> list {};

public:
	Strokes() = default;
	bool operator== (const Strokes xx) const { return list ==  xx.list; }
	auto operator<=>(const Strokes xx) const { return list <=> xx.list; }

	Strokes(Stroke);
	Strokes(std::span<const Stroke>);
	Strokes(std::initializer_list<Stroke>);
	Strokes(std::string);

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
	Brief(Strokes, std::string);
	Brief(std::string, Strokes);

	Brief(Brief, std::string);
	Brief(std::string, Brief);

	Brief& operator+=(Brief);
	Brief& operator|=(Brief);

	using Modifier = std::function<Brief(Brief)>;
	Brief& operator|=(Modifier);

private:
	void appendText(std::string);
	void normalize();
};

using Modifier = Brief::Modifier;

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
Brief   operator|(Brief  , Modifier);

// Subset of keys:
Stroke  operator&(Stroke , Stroke );

// Maybe a bit of a hack:
Brief operator+(Brief, Glue_Arg);
Brief operator+(Glue_Arg, Brief);

const auto NoStroke = Stroke {};
const auto NoStrokes = Strokes {};
const auto NoBrief = Brief {};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string toString(Stroke);
std::string toString(Strokes);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno

#include "steno_dictionary.hh"
