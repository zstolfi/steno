#include "steno.hh"
#include <algorithm>
#include <array>
#include <cctype>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(std::string_view str) {
	using State = Key;
	auto const Begin = State::Num;
	auto const End = State(int(State::_Z)+1);
	auto next = [] (State s) { return (s == End)? End: State(int(s)+1); };

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
	for (State state {Begin}; char c : str) if (!std::isspace(c)) {
		valid = true;
		if (state == End) valid = false;
		auto accept = [&] (State key, char keyChar, char numChar = '\0') {
			bool match = false;
			if (match |= c == numChar) this->set(Key::Num);
			if (match |= c == keyChar) this->set(key), state = next(key);
			return match;
		};
		switch (state) {
			using enum State;
			case Num:if (accept(Num,'#', '#')); else
			case S_: if (accept(S_, 'S', '1')); else
			case T_: if (accept(T_, 'T', '2')); else
			case K_: if (accept(K_, 'K'     )); else
			case P_: if (accept(P_, 'P', '3')); else
			case W_: if (accept(W_, 'W'     )); else
			case H_: if (accept(H_, 'H', '4')); else
			case R_: if (accept(R_, 'R'     )); else
			case A : if (accept(A , 'A', '5')); else
			/*    */ if (accept(O , 'O', '0')); else
			/*    */ if (accept(x , '*'     )); else
			/*    */ if (accept(E , 'E'     )); else
			/*    */ if (accept(U , 'U'     )); else
			/*    */ if (c == '-') state = _F;
			/*    */ else valid = false;
			break;
			case O : if (accept(O , 'O', '0')); else
			case x : if (accept(x , '*'     )); else
			case E : if (accept(E , 'E'     )); else
			case U : if (accept(U , 'U'     )); else
			case _F: if (accept(_F, 'F', '6')); else
			case _R: if (accept(_R, 'R'     )); else
			case _P: if (accept(_P, 'P', '7')); else
			case _B: if (accept(_B, 'B'     )); else
			case _L: if (accept(_L, 'L', '8')); else
			case _G: if (accept(_G, 'G'     )); else
			case _T: if (accept(_T, 'T', '9')); else
			case _S: if (accept(_S, 'S'     )); else
			case _D: if (accept(_D, 'D'     )); else
			case _Z: if (accept(_Z, 'Z'     )); else
			default: valid = false;
		}
		if (!valid) break;
	}
	if (!valid) failConstruction(str);
}

Stroke::Stroke(FromBits_Arg, std::bitset<23> b) {
	// 0b00011000100001110100000
	// = #STKPWHRAO*EUFRPBLGTSDZ
	for (unsigned i=0; i<b.size(); i++) this->bits[i] = b[22 - i];
}

Stroke::Stroke(FromBitsReversed_Arg, std::bitset<23> b) {
	// 0b00000101110000100011000
	// = ZDSTGLBPRFUE*OARHWPKTS#
	for (unsigned i=0; i<b.size(); i++) this->bits[i] = b[i];
}

bool Stroke::failed() const {
	return this->keys.FailedConstruction;
}

bool Stroke::get(Key k) const {
	return this->bits[static_cast<int>(k)];
}

Stroke Stroke::set(Key k) {
	this->bits[static_cast<int>(k)] = true;
	return *this;
}

Stroke Stroke::unset(Key k) {
	this->bits[static_cast<int>(k)] = false;
	return *this;
}

Stroke Stroke::operator+=(Stroke other) {
	this->bits |= other.bits;
	return *this;
}

Stroke Stroke::operator-=(Stroke other) {
	bool failState = this->keys.FailedConstruction;
	this->bits &= ~other.bits;
	this->keys.FailedConstruction = failState;
	return *this;
}

Stroke Stroke::operator&=(Stroke other) {
	bool failState = this->keys.FailedConstruction;
	this->bits &= other.bits;
	this->keys.FailedConstruction = failState;
	return *this;
}

void Stroke::failConstruction(std::string_view str) {
	if (str != "") std::cout << str << "\n";
	this->keys.FailedConstruction = true;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Strokes::Strokes(Stroke x) {
	this->list = List_t {x};
}

Strokes::Strokes(std::span<const Stroke> span) {
	this->list = List_t (span.begin(), span.end());
}

Strokes::Strokes(std::initializer_list<Stroke> il) {
	this->list = List_t (il.begin(), il.end());
}

Strokes::Strokes(std::string_view str) {
	auto push = [&](unsigned i, unsigned j) {
		// if (i == j) return false;
		this->list.emplace_back(str.substr(i, j-i));
		if (this->list.back().keys.FailedConstruction) return false;
		return true;
	};

	signed i=0, j=0;
	while (j=str.find('/', i), j!=str.npos) {
		if (push(i, j) == false) return;
		i = j+1;
	}
	push(i, str.size());
}

bool Strokes::failed() const {
	return std::any_of(
		list.begin(), list.end(),
		[](auto x) { return x.failed(); }
	);
}

Stroke& Strokes::operator[](std::size_t i) /* */ { return this->list[i]; }
Stroke  Strokes::operator[](std::size_t i) const { return this->list[i]; }

Strokes& Strokes::append(Stroke x) {
	this->list.insert(this->list.end(), x);
	return *this;
}

Strokes& Strokes::prepend(Stroke x) {
	this->list.insert(this->list.begin(), x);
	return *this;
}

Strokes& Strokes::append(Strokes xx) {
	this->list.insert(this->list.end(), xx.list.begin(), xx.list.end());
	return *this;
}

Strokes& Strokes::prepend(Strokes xx) {
	this->list.insert(this->list.begin(), xx.list.begin(), xx.list.end());
	return *this;
}

Strokes& Strokes::operator|=(Strokes xx) {
	return append(xx);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Brief::Brief(std::string_view str, Strokes xx): strokes{xx}, text{str} { normalize(); }
Brief::Brief(std::string_view str, Brief b): strokes{b.strokes}, text{str} { normalize(); }

bool Brief::failed() const {
	return std::any_of(
		strokes.list.begin(), strokes.list.end(),
		[](auto xx) { return xx.failed(); }
	);
}

Brief& Brief::operator+=(Brief other) {
	if (this->strokes.list.empty()) this->strokes = other.strokes;
	else if  (!other.strokes.list.empty()) {
		this->strokes.list.back() += other.strokes.list.front();
		std::copy(
			other.strokes.list.begin()+1, other.strokes.list.end(),
			std::back_inserter(this->strokes.list)
		);
	}
	appendText(other.text);
	return *this;
}

Brief& Brief::operator|=(Brief other) {
	this->strokes |= other.strokes;
	appendText(other.text);
	return *this;
}

void Brief::appendText(std::string str) {
	if (this->text.empty()) { this->text = str; return; }
	if (str.empty()) return;

	bool endGlue = this->text.back() == '~';
	bool startGlue = str.front() == '~';

	if (!startGlue && !endGlue) this->text += ' ' + str;
	else {
		if (endGlue) this->text.pop_back();
		this->text.insert(this->text.size(), str, startGlue? 1: 0);
	}
}

void Brief::normalize() {
	// Remove empty strokes.
	this->strokes.list.erase(
		std::remove(this->strokes.list.begin(), this->strokes.list.end(), NoStroke),
		this->strokes.list.end()
	);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke  operator+(Stroke  x , Stroke  y ) { return x += y ; }
Strokes operator+(Strokes xx, Stroke  y ) { return xx.list.back() += y; }
Strokes operator+(Stroke  x , Strokes yy) { return yy.list.front() += x; }
Brief   operator+(Brief   a , Brief   b ) { return a += b; }
Brief   operator+(Strokes xx, Brief   b ) { return Brief {"", xx} += b; }
Brief   operator+(Brief   b , Strokes xx) { return b += Brief {"", xx}; }
Brief   operator+(std::string s, Brief b) {
	if (s=="~") { b.text = s + b.text; return b; }
	else return Brief {s, {"-"}} += b;
}
Brief   operator+(Brief b, std::string s) {
	if (s=="~") { b.text = b.text + s; return b; }
	else return b += Brief {s, {"-"}};
}

Stroke  operator-(Stroke  x , Stroke  y ) { return x -= y; }
Strokes operator-(Strokes xx, Stroke  y ) { return xx.list.back() -= y; }

Strokes operator|(Stroke  x , Stroke  y ) { return Strokes {x, y}; }
Strokes operator|(Strokes xx, Stroke  y ) { return xx.append(y); }
Strokes operator|(Stroke  x , Strokes yy) { return yy.prepend(x); }
Strokes operator|(Strokes xx, Strokes yy) { return xx.append(yy); }
Brief   operator|(Brief   a , Brief   b ) { return a |= b; }
Brief   operator|(Strokes xx, Brief   b ) { return Brief {"", xx} |= b; }
Brief   operator|(Brief b   , Strokes xx) { return b |= Brief {"", xx}; }

Stroke  operator&(Stroke  x , Stroke  y ) { return x &= y; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string toString(Stroke x) {
	if (x.keys.OpenLeft) {
		x.keys.OpenLeft = false;
		auto result = '~' + toString(x);
		auto i = result.find(' ');
		if (i != result.npos) result.erase(i, 1);
		return result;
	}

	if (x.keys.OpenRight) {
		x.keys.OpenRight = false;
		auto result = toString(x) + '~';
		auto i = result.rfind(' ');
		if (i != result.npos) result.erase(i, 1);
		return result;
	}

	std::string result = "#STKPWHRAO*EUFRPBLGTSDZ ";
	for (unsigned i=0; i<result.size(); i++) {
		if (x.bits[i] == false) result[i] = ' ';
	}
	if (!x.keys.A && !x.keys.O && !x.keys.x && !x.keys.E && !x.keys.U) {
		result[10] = '-';
	}
	return result;
}

std::string toString(Strokes xx) {
	std::string result = "";
	for (int i=0; auto stroke : xx.list) {
		if (i++) result += '/';
		result += toString(stroke);
	}
	return result;
}

std::ostream& operator<<(std::ostream& os, Stroke x) {
	return os << toString(x);
}

std::ostream& operator<<(std::ostream& os, Strokes xx) {
	return os << toString(xx);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
