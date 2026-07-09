/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
|* ~  C++ Steno Library                                                     ~ *|
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ https://en.wikipedia.org/wiki/Stenotype ~~ */

//   The C++ Steno Library provides classes for three functionalities: keyboard,
// text, and language. This header file includes all classes (types) shown in
// the following diagram, as well as utility classes (Issues, Format, etc.)

//   Keyboard classes concern the English stenotype layout. This is a special
// 23-key keyboard which is used the same way one would play chords on a piano.

//   Text classes contain the data for sentence fragments and punctuation. They
// require a context to be manipulated, i.e. which languages rules to follow.

//   Language classes contain region-specific rules for orthography. This allows
// international developers to add in support for their own languages.



// The following diagram can be read like so:
// -	"Dictionary is a struct that has a list of Briefs"
// -	"Speech is a struct that has a list of Tokens and Context"
// -	"Token is a union that has either a Word or a Signal"

/* ┌─────────────────┬─────────────────────────────────────────┬────────────┐ *\
|*                                                                            *|
|*              ┌──────────┐                                   .              *|
|*              │Dictionary│                                                  *|
|*              └──────────┘                                   .              *|
|*                   ▲                                                        *|
|*                   │                                         .              *|
|*                   ≡                                                        *|
|*                   │                                         .              *|
|*                ┌─────┐                       ┌──────┐                      *|
|*                │Brief│                       │Speech│       .              *|
|*                └─────┘                       └──────┘                      *|
|*                   ▲                             ▲           .              *|
|*                   │                             │                          *|
|*       ┌───────────┴───────────┐     ┌───────────┴───────────┐              *|
|*       │                       │     │                       │              *|
|*  ┌──────────┐     .        ┌──────┐ │                   ┌───────┐          *|
|*  │StrokeList│              │Phrase│ │                   │Context│          *|
|*  └──────────┘     .        └──────┘ │                   └───────┘          *|
|*       ▲                         ▲   │                       ▲              *|
|*       │           .             │   │                       │              *|
|*       ≡                         ≡   ≡               ┌───────┴───────┐      *|
|*       │           .             │   │               │       .       │      *|
|*    ┌──────┐                    ┌─────┐           ┌─────┐         ┌──────┐  *|
|*    │Stroke│       .            │Token│           │State│    .    │Locale│  *|
|*    └──────┘                    └─────┘           └─────┘         └──────┘  *|
|*       ▲           .               ▲                         .              *|
|*       │                           ║                                        *|
|*       ≡           .       ╓───────╨───────╖                 .              *|
|*       │                   ║               ║                                *|
|*     ┌───┐         .     ┌────┐         ┌──────┐             .              *|
|*     │Key│               │Word│         │Signal│                            *|
|*     └───┘         .     └────┘         └──────┘             .              *|
|*                                                                            *|
|* └────Keyboard─────┴───────────────────Text──────────────────┴──Language──┘ *|
|*                                                                            *|
|*  ┌───┐          ▲            ▲           │                                 *|
|*  │   │ type     │ struct     ║ union     ≡ container                       *|
\*  └───┘          │            ║           │                                 */

#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <bit>
#include <bitset>
#include <vector>
#include <deque>
#include <span>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <cassert>

namespace steno {

/* ~~ Library Settings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef STENO_DEFAULT_LOCALE
	constexpr char const* DefaultLocale {STENO_DEFAULT_LOCALE};
#else
	constexpr char const* DefaultLocale {"en-US"};
#endif

/* ~~ Utilities ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr struct FromBits_Arg         {} FromBits         {};
constexpr struct FromBitsReversed_Arg {} FromBitsReversed {};

template <class T>
struct Issues : std::vector<T> {
	using std::vector<T>::vector;
	operator bool() const { return !this->empty(); }
};

template <class T>
class Expected {/* TODO */};

/* ~~ Key ID's ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Keys are bit-fields that identify their part of the Stroke::m_bits integer.
// Their order is based on the physical layout of a steno keyboard.

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
//	Mark = 1<<8, OpenLeft = 1<<7, OpenRight = 1<<6,
//	Fail = 1<<0,
};

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Strokes represent a subset of the steno keyboard pressed simultaneously.
// They can be manipulated like bitsets, or treated as a container of Keys.

class Stroke {
	uint32_t m_bits
	//  #STKPWHRAO*EUFRPBLGTSDZ !~~      X
	= 0b00000000000000000000000'00000000'0;
	//  └──────────┬──────────┘ └──┬───┘ └─ fail-bit
	//            keys        flags/resv'd

public:
	static constexpr unsigned KeyCount = 23;
	static constexpr unsigned PadCount = 9;
	static_assert(KeyCount + PadCount == 32);

	// Default construction/assignment
	Stroke() = default;
	Stroke(Stroke const&) = default;
	Stroke& operator=(Stroke const&) = default;

	// Class constructors
	constexpr Stroke(Key);
	constexpr Stroke(std::string_view);
	constexpr Stroke(FromBits_Arg, std::bitset<23> const);
	constexpr Stroke(FromBitsReversed_Arg, std::bitset<23> const);
	template <std::input_iterator I> constexpr Stroke(I, I);

	// Getters and Setters
	class Reference;
	class Iterator;
	uint32_t raw() const;
	bool get(Key) const;
	Stroke& set(Key, bool = true);
	Stroke& unset(Key);
	Reference operator[](Key);
	bool operator[](Key) const;
	Stroke& clear();

	// Range-for compatibility
	Iterator begin() const;
	Iterator end() const;

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

	// Fail-state query
	Issues<Key> issues() const;
	operator bool() const;

	// Key proxy class
	class Reference {
		friend class Stroke;
		Stroke* parent;
		Key key;

	public:
		Reference(Reference const&) = default;
		Reference(Stroke* p, Key k): parent{p}, key{k} {}
		operator bool() const;
		Reference& operator=(bool);
		Reference& operator=(Reference const&);
	};

	class Iterator {
		friend class Stroke;
		// Matches the bits of this Stroke.
		// The leading bit => the Key the iterator "points" to.
		uint32_t m_bits = 0;

	public:
		Iterator() = default;
		Iterator(Stroke const& s): m_bits{s.m_bits} {}
		bool operator==(Iterator const&) const;
		Iterator& operator++();
		Iterator operator++(int);
		Key operator*() const;
		using difference_type = int;
		using value_type = Key;
	};
	// TODO: Figure out why this assert doesn't work.
//	static_assert(std::indirectly_readable<Iterator>);

private:
	static constexpr auto
	//            #STKPWHRAO*EUFRPBLGTSDZ !~~      X
	FailBit   = 0b00000000000000000000000'00000000'1,
	FlagsMask = 0b00000000000000000000000'11111111'1;

	uint32_t getFlags() const;
	void setFlags(uint32_t);
};

// Key promotion
Stroke operator~(Key);
Stroke operator+(Key, Key);
Stroke operator-(Key, Key);
Stroke operator&(Key, Key);
Stroke operator^(Key, Key);

/* ~~ StrokeList Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// StrokeLists are sequences of strokes.

class StrokeList : public std::vector<Stroke> {
public:
	// Default construction/assignment
	using std::vector<Stroke>::vector;

	// Class constructors
	StrokeList(std::string_view);
	StrokeList(Stroke);
	StrokeList(std::span<Stroke const>);

	// Comparison
	bool operator== (StrokeList const&) const = default;
	auto operator<=>(StrokeList const&) const = default;

	// Concatenation
	StrokeList& operator|=(StrokeList);
	friend StrokeList operator|(StrokeList, StrokeList const&);

	// Fail-state query
	Issues<Stroke const*> issues() const;
	operator bool() const;

	// Vector methods
	friend void erase   (StrokeList& p, auto&& value);
	friend void erase_if(StrokeList& p, auto&& pred);

private:
	void erase_impl(auto&& value)
	{ erase(std::remove(begin(), end(), value), end()); }
	void erase_if_impl(auto&& pred)
	{ erase(std::remove_if(begin(), end(), pred), end()); }
};

// Stroke promotion
StrokeList operator|(Stroke, Stroke const&);

/* ~~ Text Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Texts are output iterators which follow orthographic rules.
// As such, they depend on some locale. This can be selected at compile time, or
// at run time.

// TODO

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Phrases are strings with embedded formatting information.
// This information modifies the context of any created Text object, such as
// instructions to capitalize the next phrase, join to the next or previous
// phrase without a space (affix), etc.

using Phrase = std::string; // TODO

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Briefs associate StrokeLists and Phrases, and can be modified like either.
// They are primarily used as Dictionary entries.

class Brief {
	StrokeList m_strokeList {};
	Phrase m_phrase {};

public:
	// Default construction/assignment
	Brief() = default;
	Brief(Brief const&) = default;
	Brief& operator=(Brief const&) = default;

	// Class constructors
	Brief(StrokeList const&, Phrase);
	Brief(Brief const&, Phrase);

	// Getters and Setters
	StrokeList /* */& strokes();
	StrokeList const& strokes() const;
	Phrase /* */& phrase();
	Phrase const& phrase() const;
	template <std::size_t I> friend auto&& get(Brief&);
	template <std::size_t I> friend auto&& get(Brief const&);
	template <std::size_t I> friend auto&& get(Brief&&);
	Brief& clear();

	// Comparison
	bool operator== (Brief const&) const = default;
	auto operator<=>(Brief const&) const = default;

	// Concatenation
	Brief& operator|=(Brief);
	friend Brief operator|(Brief, Brief const&);
	Brief& operator+=(Phrase);
	friend Brief operator+(Brief, Phrase);
	friend Brief operator+(Phrase, Brief);

	// Fail-state query
	// TODO: Figure out how to report phrase issues.
	Issues<Stroke const*> issues() const;
	operator bool() const;

private:
	Brief& normalize();

#	define GET_IMPL(Attr) \
	template <std::size_t I> auto&& get_impl() Attr {                          \
	    if constexpr (I==0) return m_strokeList;                               \
	    if constexpr (I==1) return m_phrase;                                   \
	}
	GET_IMPL(&) GET_IMPL(const&) GET_IMPL(&&)
#	undef GET_IMPL
};

// StrokeList promotion
Brief operator+(StrokeList, Phrase);
Brief operator+(Phrase, StrokeList);

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Dictionaries provide fast look-up and insertion for many Briefs.
// They behave like std::map<StrokeList, Phrase>, with Brief value_types.

class Dictionary {
	std::deque<Brief> m_entries {};

public:
	// Default construction/assignment/movement
	Dictionary() = default;
	Dictionary(Dictionary const&) = default;
	Dictionary(Dictionary&&/* */) = default;
	Dictionary& operator=(Dictionary const&) = default;
	Dictionary& operator=(Dictionary&&/* */) = default;

	// Class constructors
	// Use parseDictionary() for file type support.
	Dictionary(std::span<Brief const>);

	// Comparison
	bool operator== (Dictionary const&) const = default;
	auto operator<=>(Dictionary const&) const = default;

	// Fail-state query
	Issues<Brief const*> issues() const;
	void clean();

public:
	// Container types
	using value_type = Brief;
	using reference = Brief&;
	using const_reference = Brief const&;
	using pointer = Brief*;
	using const_pointer = Brief const*;
	using iterator = decltype(m_entries)::iterator;
	using const_iterator = decltype(m_entries)::const_iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;

	// Container methods
#	define USE(Name) \
	auto    Name()       { return m_entries.   Name(); } \
	auto    Name() const { return m_entries.   Name(); } \
	auto c##Name() const { return m_entries.c##Name(); }
	USE(begin) USE(end) USE(rbegin) USE(rend)
#	undef USE
	void swap(Dictionary& other) { std::swap(*this, other); };
	std::size_t size    () const { return m_entries.size    (); }
	std::size_t max_size() const { return m_entries.max_size(); }
	bool        empty   () const { return m_entries.empty   (); }

public:
	// Associative types
	using key_type = StrokeList;
	using mapped_type = Phrase;

	// Associative methods
	iterator insert(Brief const&);
	template <std::input_iterator I>
	Dictionary(I first, I last): m_entries(first, last) { normalize(); }
	Dictionary(std::initializer_list<Brief> il): m_entries(il) { normalize(); };
	template <std::input_iterator I>
	void insert(I i, I j) { for (I it=i; it!=j; ++it) insert(*it); }
	void insert(std::initializer_list<Brief>);
	iterator emplace(auto&& ... args) { return insert(Brief {args ... }); }
	std::size_t erase(StrokeList);
	iterator erase(const_iterator);
	iterator erase(const_iterator, const_iterator);
	void merge(Dictionary&);
	void merge(Dictionary&&);
	void clear();
	bool contains(StrokeList const&) const;
	/*  */iterator find(StrokeList const&);
	const_iterator find(StrokeList const&) const;
	/*  */iterator lower_bound(StrokeList const&);
	const_iterator lower_bound(StrokeList const&) const;
	/*  */iterator upper_bound(StrokeList const&);
	const_iterator upper_bound(StrokeList const&) const;
	std::pair<iterator, iterator>
	equal_range(StrokeList const&);
	std::pair<const_iterator, const_iterator>
	equal_range(StrokeList const&) const;

	// Map methods
	Phrase& operator[](StrokeList const&);
	Phrase const& operator[](StrokeList const&) const;
	Phrase& at(StrokeList const&);
	Phrase const& at(StrokeList const&) const;
	friend void erase   (Dictionary& p, auto&& value);
	friend void erase_if(Dictionary& p, auto&& pred);

private:
	void normalize();
	void erase_if_impl(auto&& pred)
	{ for (auto it=begin(); it!=end(); ++it) if (pred(*it)) erase(it); }
	void erase_impl(auto&& value)
	{ erase_if_impl([&] (auto y) { return value == y; }); }
};

/* ~~ String Output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Formats can be used inside a std::ostream or passed as function arguments.

// We use 'long' for ios_base::iword compatibility.
enum class Format : long {
	// Width                XX
	Packed        = 0b00'00'01,
	Wide          = 0b00'00'10,
	// Hyphen            XX
	Hyphen        = 0b00'01'00,
	NoHyphen      = 0b00'10'00,
	// Digit          XX
	Numeric       = 0b01'00'00,
	Alpha         = 0b10'00'00,

	StrokeDefault = 0b01'01'01,
	KeyDefault    = 0b00'10'00,
	// TODO: More formatting options for StrokeLists and upward
};
using enum Format;
Format operator|(Format, Format);
Format operator|=(Format&, Format);

char toChar(Key);
char toCharShift(Key);
std::string toString(Key, Format = KeyDefault);
std::string toString(Stroke, Format = StrokeDefault);
std::string toString(StrokeList const&, Format = StrokeDefault);
std::string toString(Brief const&, Format = StrokeDefault);
std::ostream& operator<<(std::ostream&, Key);
std::ostream& operator<<(std::ostream&, Stroke);
std::ostream& operator<<(std::ostream&, StrokeList const&);
std::ostream& operator<<(std::ostream&, Brief const&);

// Format as manipulator
std::ostream& operator<<(std::ostream&, Format);

/* ~~ Constexpr Declarations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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
		[&] (char c) { return in(c, " \t" "#0123456789"); }
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
			case Nm: if (accept(Nm, Key::Num,'#', '#')); else
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
	if (!valid) m_bits |= FailBit;
}

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

template <std::input_iterator I>
constexpr Stroke::Stroke(I first, I last) {
	for (auto key=first; key!=last; ++key) {
		this->m_bits |= (uint32_t)*key;
	}
}

static constexpr auto NoStroke = Stroke {};
static const/**/ auto NoStrokeList = StrokeList {};
static const/**/ auto NoBrief  = Brief  {};
static const/**/ auto NoPhrase   = Phrase   {};
// TODO: NoIssues object which acts like std::nullopt

} // namespace steno

/* ~~ Misc. STL Functionality ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <> struct std::hash<steno::Stroke>
{ std::size_t operator()(steno::Stroke const&) const; };

template <> struct std::hash<steno::StrokeList>
{ std::size_t operator()(steno::StrokeList const&) const; };

template <> struct std::tuple_size<steno::Brief>
: std::integral_constant<size_t, 2> {};

template <std::size_t I> struct std::tuple_element<I, steno::Brief>
: std::conditional<I == 0, steno::StrokeList, std::string>
{ static_assert(I < 2); };

namespace steno {
void erase   (StrokeList&     t, auto&& x) { t.erase_impl(x);    }
void erase_if(StrokeList&     t, auto&& f) { t.erase_if_impl(f); }
void erase   (Dictionary& t, auto&& x) { t.erase_impl(x);    }
void erase_if(Dictionary& t, auto&& f) { t.erase_if_impl(f); }
template <std::size_t I> auto&& get(Brief&       b) { return b.get_impl<I>(); }
template <std::size_t I> auto&& get(Brief const& b) { return b.get_impl<I>(); }
template <std::size_t I> auto&& get(Brief&&      b) { return b.get_impl<I>(); }
}

//using steno::erase;
//using steno::erase_if;
