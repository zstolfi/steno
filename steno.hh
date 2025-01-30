#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include <forward_list>
#include <span>
#include <initializer_list>


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
	bool operator==(const Stroke other) const {
		return this->keys.bits
		==     other.keys.bits;
	}
	auto operator<=>(const Stroke other) const {
		return this->keys.bits.to_ulong()
		<=>    other.keys.bits.to_ulong();
	}


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

private:
	void appendText(std::string);

	void normalize();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Dictionary {
	// Allow conflicts in translation by allowing multiple texts per entry.
	// Conflicts are rare, so I use a forward_list instead of a multimap.
	using Translation = std::forward_list<std::string>;
	std::map<Strokes, Translation> entries {};

public:
	Dictionary();
	Dictionary(std::span<Brief>);
	Dictionary(std::initializer_list<Brief>);

private:
	using Entry = decltype(entries)::value_type;

public:
	void add(Brief b);
	std::string translate(const Entry&) const;
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Multiple keys at the same time
Stroke  operator+(Stroke , Stroke );
Strokes operator+(Strokes, Stroke );
Strokes operator+(Stroke , Strokes);
Brief   operator+(Brief  , Brief  );

// Multiple strokes in order
Strokes operator|(Stroke , Stroke );
Strokes operator|(Strokes, Stroke );
Strokes operator|(Stroke , Strokes);
Strokes operator|(Strokes, Strokes);
Brief   operator|(Brief  , Brief  );

const auto NoStroke = Stroke {};
const auto NoStrokes = Strokes {};
const auto NoBrief = Brief {};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string toString(Stroke);
std::string toString(Strokes);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
