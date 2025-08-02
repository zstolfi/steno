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

namespace steno {

/* ~~ API Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr struct FromBits_Arg {} FromBits;
constexpr struct FromBitsReversed_Arg {} FromBitsReversed;

/* ~~ Key ID's ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Stroke {
	//                          keys        flags/resv'd  
	//                ┌──────────┴──────────┐ ┌──┴───┐ ┌─ fail-bit
	//                #STKPWHRAO*EUFRPBLGTSDZ !~~      X
	uint32_t bits = 0b00000000000000000000000'00000000'0;

public:
	// Default construction/assignment
	Stroke() = default;
	Stroke(Stroke const&) = default;
	Stroke& operator=(Stroke const&) = default;
	// Class constructors
	constexpr Stroke(std::string_view);
	Stroke(FromBits_Arg, std::bitset<23>);
	Stroke(FromBitsReversed_Arg, std::bitset<23>);
	// Fail-state query
	bool failed() const;
	operator bool() const;
	// Getters and Setters
	uint32_t getBits() const;
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
	// Key manipulation
	Stroke operator~() const;
	Stroke& operator+=(Stroke);
	Stroke& operator-=(Stroke);
	Stroke& operator&=(Stroke);
	Stroke& operator^=(Stroke);

private:
	uint32_t getFlags() const;
	void setFlags(uint32_t);
	void failConstruction(std::string_view = "");
};

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Phrase {
	std::vector<Stroke> strokes {};

public:
	// Default construction/assignment
	Phrase() = default;
	Phrase(Phrase const&) = default;
	Phrase& operator=(Phrase const&) = default;
	// Class constructors
	Phrase(std::string_view);
	Phrase(Stroke);
	Phrase(std::span<const Stroke>);
	Phrase(std::initializer_list<Stroke>);
	// Fail-state query
	bool failed() const;
	operator bool() const;
	// Getters and Setters
	std::vector<Stroke>& getStrokes();
	Phrase& append (Phrase);
	Phrase& prepend(Phrase);
	Stroke& operator[](std::size_t);
	Stroke  operator[](std::size_t) const;
	// Comparison
	bool operator== (Phrase const& xx) const = default;
	auto operator<=>(Phrase const& xx) const = default;
	// Phrase concatenation
	Phrase& operator|=(Phrase);

public:
	// Container specific types
	using value_type = Stroke;
	using reference = Stroke&;
	using const_reference = Stroke const&;
	using iterator = Stroke*;
	using const_iterator = Stroke const*;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;
	// Container specific methods
	Stroke*       begin();
	Stroke const* begin() const;
	Stroke const* cbegin() const;
	Stroke*       end();
	Stroke const* end() const;
	Stroke const* cend() const;
	void swap(Phrase&);
	std::size_t size() const;
	std::size_t max_size() const;
	bool empty() const;

public:
	// Sequence specific methods
	Phrase(std::size_t, steno::Stroke);
	template <std::input_iterator I>
	Phrase(I first, I last) { strokes = std::vector<Stroke> (first, last); }
};

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//struct Brief {
//	Phrase strokes {};
//	std::string text {};

//public:
//	Brief() = default;
//	Brief(std::string_view, Phrase);
//	Brief(std::string_view, Brief);

//	bool operator==(const Brief& b) const = default;

//	bool failed() const;

//	Brief& operator+=(Brief);
//	Brief& operator|=(Brief);

//private:
//	void appendText(std::string);
//	void normalize();
//};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//using Dictionary = std::map<steno::Phrase, std::string>;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Multiple keys at the same time:
Stroke operator+(Stroke, Stroke);
Phrase operator+(Phrase, Stroke);
Phrase operator+(Stroke, Phrase);
//Brief  operator+(Brief , Brief );
//Brief  operator+(Phrase, Brief );
//Brief  operator+(Brief , Phrase);
//Brief  operator+(std::string, Brief);
//Brief  operator+(Brief, std::string);

// Removing keys:
Stroke operator-(Stroke, Stroke);
Phrase operator-(Phrase, Stroke);

// Multiple strokes in order:
Phrase operator|(Stroke, Stroke);
Phrase operator|(Phrase, Stroke);
Phrase operator|(Stroke, Phrase);
Phrase operator|(Phrase, Phrase);
//Brief  operator|(Brief , Brief );
//Brief  operator|(Phrase, Brief );
//Brief  operator|(Brief , Phrase);
// Subset of keys:
Stroke  operator&(Stroke , Stroke );
// Toggling keys:
Stroke  operator^(Stroke , Stroke );

std::string toString(Key);
std::string toString(Stroke);
std::string toString(Phrase);
std::ostream& operator<<(std::ostream&, Stroke);
std::ostream& operator<<(std::ostream&, Phrase);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr Stroke::Stroke(std::string_view str) {
	enum State {
		Mk, /*!*/    Begin = Mk,
		Ol, /*~*/
		Nm, /*#*/
		S_,
		T_, K_,
		P_, W_,
		H_, R_,
		A , O ,
		x ,
		E , U ,
		_F, _R,
		_P, _B,
		_L, _G,
		_T, _S,
		_D, _Z,
		Or, /*~*/    End = Or+1
	};

	auto next = [] (State s) { return (s == End)? End: State(int(s)+1); };
	auto bitFromKey = [] (Key k) { return 31-int(k); };

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
	for (State state {Begin}; char c : str) if (c != ' ' && c != '\t') {
		valid = true;
		if (state == End) valid = false;
		auto accept = [&] (State s, Key k, char keyChar, char numChar = '\0') {
			if (c == keyChar || c == numChar) {
				if (c == numChar) this->bits |= 1 << bitFromKey(Key::Num);
				if (c == keyChar) this->bits |= 1 << bitFromKey(k);
				state = next(s);
				return true;
			}
			return false;
		};
		switch (state) {
			using enum State;
			case Mk: if (accept(End,Key::Mark    ,'!')); else
			case Ol: if (accept(Ol, Key::OpenLeft,'~')); else
			case Nm: if (accept(Nm, Key::Num,'#'     )); else
			case S_: if (accept(S_, Key::S_, 'S', '1')); else
			case T_: if (accept(T_, Key::T_, 'T', '2')); else
			case K_: if (accept(K_, Key::K_, 'K'     )); else
			case P_: if (accept(P_, Key::P_, 'P', '3')); else
			case W_: if (accept(W_, Key::W_, 'W'     )); else
			case H_: if (accept(H_, Key::H_, 'H', '4')); else
			case R_: if (accept(R_, Key::R_, 'R'     )); else
			case A : if (accept(A , Key::A , 'A', '5')); else
			/*    */ if (accept(O , Key::O , 'O', '0')); else
			/*    */ if (accept(x , Key::x , '*'     )); else
			/*    */ if (accept(E , Key::E , 'E'     )); else
			/*    */ if (accept(U , Key::U , 'U'     )); else
			/*    */ if (c == '-') state = _F;
			/*    */ else valid = false;
			break;
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
			case Or: if (accept(Or,Key::OpenRight,'~')); else
			default: valid = false;
		}
		if (!valid) break;
	}
	if (!valid) failConstruction(str);
}

namespace KeyUnit {
	constexpr static steno::Stroke
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

const auto NoStroke = Stroke {};
const auto NoPhrase = Phrase {};
//const auto NoBrief = Brief {};

} // namespace steno

template <> struct std::hash<steno::Stroke>
{ std::size_t operator()(steno::Stroke const&) const; };
