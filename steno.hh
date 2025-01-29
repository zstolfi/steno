#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <span>
#include <initializer_list>
#include <compare>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct FromBits_t {};
constexpr FromBits_t FromBits;

struct FromBitsReversed_t {};
constexpr FromBitsReversed_t FromBitsReversed;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Stroke {
	union {
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
		};
	} keys;

public:
	Stroke() = default;
	auto operator<=>(const Stroke x) const
	{ return keys.bits.to_ulong() <=> x.keys.bits.to_ulong(); }


	Stroke(std::string);
	Stroke(FromBits_t, std::bitset<23>);
	Stroke(FromBitsReversed_t, std::bitset<23>);

	Stroke operator+=(Stroke);

private:
	void failConstruction();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Strokes {
	std::vector<Stroke> list {};

public:
	Strokes() = default;
	auto operator<=>(const Strokes xx) const { return list <=> xx.list; }

	Strokes(Stroke);
	Strokes(std::span<Stroke>);
	Strokes(std::initializer_list<Stroke>);
	Strokes(std::string);

	Stroke& operator[](std::size_t);
	Stroke  operator[](std::size_t) const;

	Strokes& append(Stroke);
	Strokes& prepend(Stroke);
	Strokes& append(const Strokes&);
	Strokes& prepend(const Strokes&);
	Strokes& operator|=(const Strokes&);

private:
	using List_t = decltype(list);
	void normalize();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// struct Brief {

// };

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Multiple keys at the same time
Stroke  operator+(Stroke , Stroke );
Strokes operator+(Strokes, Stroke );
Strokes operator+(Stroke , Strokes);

// Multiple strokes in order
Strokes operator|(Stroke , Stroke );
Strokes operator|(Strokes, Stroke );
Strokes operator|(Stroke , Strokes);
Strokes operator|(Strokes, Strokes);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const auto NoStroke = Stroke {};
const auto NoStrokes = Strokes {};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
