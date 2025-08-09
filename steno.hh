#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <bitset>
#include <vector>
#include <map>
#include <span>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <cstdint>

namespace steno {

/* ~~ API Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr struct FromBits_Arg         {} FromBits         {};
constexpr struct FromBitsReversed_Arg {} FromBitsReversed {};

/* ~~ Key ID's ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

enum class Key : uint32_t {
	// Number Bar
	Num = 1u<<31,
	// Initial Consonants
	S_ = 1u<<30,
	T_ = 1u<<29, K_ = 1u<<28,
	P_ = 1u<<27, W_ = 1u<<26,
	H_ = 1u<<25, R_ = 1u<<24,
	// Vowels & Asterisk
	A  = 1u<<23, O  = 1u<<22,
	x  = 1u<<21,
	E  = 1u<<20, U  = 1u<<19,
	// Final Consonants
	_F = 1u<<18, _R = 1u<<17,
	_P = 1u<<16, _B = 1u<<15,
	_L = 1u<<14, _G = 1u<<13,
	_T = 1u<<12, _S = 1u<<11,
	_D = 1u<<10, _Z = 1u<< 9,
	// Flags
//	Mark = 1<<8, OpenLeft = 1<<7, OpenRight = 1<<0,
};

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Stroke {
	uint32_t m_bits
	//  #STKPWHRAO*EUFRPBLGTSDZ !~~      X
	= 0b00000000000000000000000'00000000'0;
	//  └──────────┬──────────┘ └──┬───┘ └─ fail-bit
	//            keys        flags/resv'd  

	static constexpr unsigned KeyCount = 23;
	static constexpr unsigned PadCount = 9;
	static_assert(KeyCount + PadCount == 32);

public:
	// Default construction/assignment
	Stroke() = default;
	Stroke(Stroke const&) = default;
	Stroke& operator=(Stroke const&) = default;
	// Class constructors
	constexpr Stroke(Key);
	constexpr Stroke(std::string_view);
	constexpr Stroke(FromBits_Arg, std::bitset<23> const);
	constexpr Stroke(FromBitsReversed_Arg, std::bitset<23> const);
	// Fail-state query
	bool failed() const;
	operator bool() const;
	// Getters and Setters
	uint32_t raw() const;
	bool get(Key) const;
	Stroke& set(Key, bool = true);
	Stroke& unset(Key);
	// Key proxy class
	class Reference {
		Stroke* parent;
		Key key;

	public:
		Reference(Reference const&) = default;
		Reference(Stroke* p, Key k): parent{p}, key{k} {}
		operator bool() const;
		Reference& operator=(bool);
		Reference& operator=(Reference const&);
	};
	// Subscript operator
	bool operator[](Key) const;
	Reference operator[](Key);
	// Comparison
	bool operator==(Stroke const&) const = default;
	auto operator<=>(Stroke const&) const = default;
	template <class T> friend struct std::hash;
	// Key manipulation
	Stroke operator~() const;
	Stroke& operator+=(Stroke const&);
	Stroke& operator-=(Stroke const&);
	Stroke& operator&=(Stroke const&);
	Stroke& operator^=(Stroke const&);
	friend Stroke operator+(Stroke, Stroke const&);
	friend Stroke operator-(Stroke, Stroke const&);
	friend Stroke operator&(Stroke, Stroke const&);
	friend Stroke operator^(Stroke, Stroke const&);

private:
	uint32_t getFlags() const;
	void setFlags(uint32_t);
	void failConstruction(std::string_view = "");
};

// Key promotion
Stroke operator~(Key);
Stroke operator+(Key, Key);
Stroke operator-(Key, Key);
Stroke operator&(Key, Key);
Stroke operator^(Key, Key);

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Phrase {
	std::vector<Stroke> m_strokes {};

public:
	// Default construction/assignment/movement
	Phrase() = default;
	Phrase(Phrase const&) = default;
	Phrase& operator=(Phrase const&) = default;
	// Class constructors
	Phrase(std::string_view);
	Phrase(Stroke);
	Phrase(std::span<Stroke const>);
	// Fail-state query
	bool failed() const;
	operator bool() const;
	// Comparison
	bool operator== (Phrase const&) const = default;
	auto operator<=>(Phrase const&) const = default;
	template <class T> friend struct std::hash;
	// Concatenation
	Phrase& operator|=(Phrase);
	friend Phrase operator|(Phrase, Phrase const&);

public:
	// Container specific types
	using value_type = Stroke;
	using reference = Stroke&;
	using const_reference = Stroke const&;
	using iterator = decltype(m_strokes)::iterator;
	using const_iterator = decltype(m_strokes)::const_iterator;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;
	// Container specific methods
	auto begin ()       { return m_strokes.begin (); }
	auto begin () const { return m_strokes.begin (); }
	auto cbegin() const { return m_strokes.cbegin(); }
	auto end   ()       { return m_strokes.end   (); }
	auto end   () const { return m_strokes.end   (); }
	auto cend  () const { return m_strokes.cend  (); }
	void swap(Phrase& other) { std::swap(*this, other); };
	std::size_t size    () const { return m_strokes.size    (); }
	std::size_t max_size() const { return m_strokes.max_size(); }
	bool        empty   () const { return m_strokes.empty   (); }

public:
	// Sequence specific methods
	Phrase(std::initializer_list<Stroke> il): m_strokes(il) {};
	Phrase(std::size_t n, Stroke t): m_strokes(n, t) {}
	template <std::input_iterator I>
	Phrase(I first, I last): m_strokes(first, last) {}
	auto emplace(auto&& ... args) { return m_strokes.emplace(args ... ); }
	auto insert (auto&& ... args) { return m_strokes.insert (args ... ); }
	auto erase  (auto&& ... args) { return m_strokes.erase  (args ... ); }
	auto clear  (auto&& ... args) { return m_strokes.clear  (args ... ); }
	auto assign (auto&& ... args) { return m_strokes.assign (args ... ); }
	// Vector specific methods
	auto& front  ()       { return m_strokes.front(); }
	auto& front  () const { return m_strokes.front(); }
	auto& back   ()       { return m_strokes.back (); }
	auto& back   () const { return m_strokes.back (); }
	auto  emplace_back(auto&& ... args)   { m_strokes.emplace_back(args ... ); }
	auto  push_back   (auto&& ... args)   { m_strokes.push_back   (args ... ); }
	auto  pop_back  ()                    { m_strokes.pop_back    ();          }
	auto& operator[](std::size_t n)       { return m_strokes[n];               }
	auto& operator[](std::size_t n) const { return m_strokes[n];               }
	auto& at        (std::size_t n)       { return m_strokes.at(n);            }
	auto& at        (std::size_t n) const { return m_strokes.at(n);            }
};

// Stroke promotion
Phrase operator|(Stroke, Stroke const&);

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Brief {
	Phrase m_phrase {};
	std::string m_text {};

public:
	// Default construction/assignment/movement
	Brief() = default;
	Brief(Brief const&) = default;
	Brief& operator=(Brief const&) = default;
	// Class constructors
	Brief(Phrase const&, std::string_view);
	Brief(Brief const&, std::string_view);
	// Fail-state query
	bool failed() const;
	operator bool() const;
	// Getters and Setters
	Phrase&       phrase();
	Phrase const& phrase() const;
	std::string&       text();
	std::string const& text() const;
	template <std::size_t I> friend auto&& get(Brief&);
	template <std::size_t I> friend auto&& get(Brief const&);
	template <std::size_t I> friend auto&& get(Brief&&);
	// Comparison
	bool operator== (Brief const&) const = default;
	auto operator<=>(Brief const&) const = default;
	// Concatenation
	Brief& operator|=(Brief);
	friend Phrase operator|(Phrase, Phrase const&);
	Brief& operator+=(std::string_view);
	friend Brief operator+(Brief, std::string_view);
	friend Brief operator+(std::string_view, Brief);

private:
	Brief& normalize();
	template <std::size_t I> auto&& get_impl() &
	{ if constexpr (I==0) return m_phrase; if constexpr (I==1) return m_text; }
	template <std::size_t I> auto&& get_impl() const&
	{ if constexpr (I==0) return m_phrase; if constexpr (I==1) return m_text; }
	template <std::size_t I> auto&& get_impl() &&
	{ if constexpr (I==0) return m_phrase; if constexpr (I==1) return m_text; }
};

// Phrase promotion
Brief operator+(Phrase, std::string_view);
Brief operator+(std::string_view, Phrase);

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//using Dictionary = std::map<steno::Phrase, std::string>;

/* ~~ String Output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//std::string toString(Key);
//std::string toString(Stroke);
//std::string toString(Phrase);
//std::ostream& operator<<(std::ostream&, Stroke);
//std::ostream& operator<<(std::ostream&, Phrase);

/* ~~ Constexpr Declarations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr Stroke::Stroke(Key k) {
	this->m_bits = (uint32_t)k;
}

constexpr Stroke::Stroke(FromBits_Arg, std::bitset<23> const b) {
	for (unsigned i=0; i<b.size(); i++) if (b[i]) {
		this->m_bits |= 1 << (i + Stroke::PadCount);
	}
}

constexpr Stroke::Stroke(FromBitsReversed_Arg, std::bitset<23> const b) {
	for (unsigned i=0; i<b.size(); i++) if (b[22-i]) {
		this->m_bits |= 1 << (i + Stroke::PadCount);
	}
}

constexpr Stroke::Stroke(std::string_view str) {
	enum State {
//		Mk, /*!*/
//		Ol, /*~*/
		Nm, /*#*/
		S_, T_, K_, P_, W_, H_, R_,
		A , O , x , E , U ,
		_F, _R, _P, _B, _L, _G, _T, _S, _D, _Z,
//		Or, /*~*/
		Begin = Nm, End = _Z+1
	};

	auto next = [] (State s) { return (s == End)? End: State(int(s)+1); };
	auto in = [] (char c, std::string_view s) { return s.find(c) != s.npos; };
	bool const Numeric = std::all_of(
		str.begin(), str.end(),
		[&] (char c) { return in(c, " \t" "0123456789"); }
	);

	//   On the left is every state's possible next valid input. This creates
	// a broken triangle we can play billiards on to parse our string.
	//   On the right we play this game with the example input "SPROUTS".
	// Advancing in the X direction <=> finding next valid input.
	// Advancing in the Y direction <=> advancing in the switch statement.

	// S_:    STKPWHRAO*EU-                     █TKPWHRAO*EU-         
	// T_:     TKPWHRAO*EU-                     █──█WHRAO*EU-         
	// K_:      KPWHRAO*EU-                       K│WHRAO*EU-         
	// P_:       PWHRAO*EU-                        │WHRAO*EU-         
	// W_:        WHRAO*EU-                        █──█AO*EU-         
	// H_:         HRAO*EU-                          H│AO*EU-         
	// R_:          RAO*EU-                           │AO*EU-         
	// A :           AO*EU-                           █─█*EU-         
	// O :            O*EUFRPBLGTSDZ                    │*EUFRPBLGTSDZ
	// x :             *EUFRPBLGTSDZ                    █──█FRPBLGTSDZ
	// E :              EUFRPBLGTSDZ                      E│FRPBLGTSDZ
	// U :               UFRPBLGTSDZ                       │FRPBLGTSDZ
	// _F:                FRPBLGTSDZ                       █──────█SDZ
	// _R:                 RPBLGTSDZ                         RPBLG│SDZ
	// _P:                  PBLGTSDZ                          PBLG│SDZ
	// _B:                   BLGTSDZ                           BLG│SDZ
	// _L:                    LGTSDZ                            LG│SDZ
	// _G:                     GTSDZ                             G│SDZ
	// _T:                      TSDZ                              │SDZ
	// _S:                       SDZ                              ██DZ
	// _D:                        DZ                               █──End;
	// _Z:                         Z                                 Z

	bool valid = false;
	for (State state {Begin}; char c : str) if (!in(c, " \t")) {
		valid = true;
		auto require = [&] (bool condition) { return !(valid &= condition); };
		auto accept = [&] (State s, Key k, char cKey, char cNum = '\0') {
			bool match = (c == cKey) || (c == cNum);
			if (c == cKey) this->m_bits |= (uint32_t)k;
			if (c == cNum) this->m_bits |= (uint32_t)k | (uint32_t)Key::Num;
			if (match) state = next(s);
			return match;
		};
		switch (state) {
			using enum State;
//			case Mk: if (accept(End,Key::Mark    ,'!')); else
//			case Ol: if (accept(Ol, Key::OpenLeft,'~')); else
			case Nm: if (accept(Nm, Key::Num,'#'     )); else
			case S_: if (accept(S_, Key::S_, 'S', '1')); else
			case T_: if (accept(T_, Key::T_, 'T', '2')); else
			case K_: if (accept(K_, Key::K_, 'K'     )); else
			case P_: if (accept(P_, Key::P_, 'P', '3')); else
			case W_: if (accept(W_, Key::W_, 'W'     )); else
			case H_: if (accept(H_, Key::H_, 'H', '4')); else
			case R_: if (accept(R_, Key::R_, 'R'     )); else
			case A : if (accept(A , Key::A , 'A', '5')); else
			if (c == '-') state = _F;                    else
			if (require(Numeric || in(c, "O*EU" "0")));  else
			case O : if (accept(O , Key::O , 'O', '0')); else
			case x : if (accept(x , Key::x , '*'     )); else
			case E : if (accept(E , Key::E , 'E'     )); else
			case U : if (accept(U , Key::U , 'U'     )); else
			case _F: if (accept(_F, Key::_F, 'F', '6')); else
			case _R: if (accept(_R, Key::_R, 'R'     )); else
			case _P: if (accept(_P, Key::_P, 'P', '7')); else
			case _B: if (accept(_B, Key::_B, 'B'     )); else
			case _L: if (accept(_L, Key::_L, 'L', '8')); else
			case _G: if (accept(_G, Key::_G, 'G'     )); else
			case _T: if (accept(_T, Key::_T, 'T', '9')); else
			case _S: if (accept(_S, Key::_S, 'S'     )); else
			case _D: if (accept(_D, Key::_D, 'D'     )); else
			case _Z: if (accept(_Z, Key::_Z, 'Z'     )); else
//			case Or: if (accept(Or,Key::OpenRight,'~')); else
			default: valid = false;
		}
		if (!valid) break;
	}
	if (!valid) failConstruction(str);
}

static constexpr auto NoStroke = Stroke {};
static constexpr auto NoPhrase = Phrase {};
static constexpr auto NoBrief  = Brief  {};

} // namespace steno

/* ~~ Misc. STL Functionality ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <> struct std::hash<steno::Stroke>
{ std::size_t operator()(steno::Stroke const&) const; };

template <> struct std::hash<steno::Phrase>
{ std::size_t operator()(steno::Phrase const&) const; };

template <> struct std::tuple_size<steno::Brief>
: std::integral_constant<size_t, 2> {};

template <std::size_t I> struct std::tuple_element<I, steno::Brief>
: std::conditional<I == 0, steno::Phrase, std::string>
{ static_assert(I < 2); };

namespace steno {
template <std::size_t I> auto&& get(Brief&       b) { return b.get_impl<I>(); }
template <std::size_t I> auto&& get(Brief const& b) { return b.get_impl<I>(); }
template <std::size_t I> auto&& get(Brief&&      b) { return b.get_impl<I>(); }
}