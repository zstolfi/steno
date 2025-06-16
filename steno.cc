#include "steno.hh"
#include <algorithm>
#include <string_view>
#include <array>
#include <cassert>

namespace /*detail*/ {
	const std::string Whitespace = " \t\r\n";
	// Flags:
	const char Hash = '#', Dash = '-', Mark = '!', Tilde = '~';
	//                               Left   Mid   Right
	// Keys:                        ┌──┴──┐┌─┴─┐┌───┴────┐
	const std::string StenoOrder = "STKPWHRAO*EUFRPBLGTSDZ";
	const std::string Left   = StenoOrder.substr( 0,  7);
	const std::string Middle = StenoOrder.substr( 7,  5);
	const std::string Right  = StenoOrder.substr(12, 10);
	// Keys when the number bar is pressed:
	const std::string StenoNumbers = "12K3W4R50*EU6R7B8G9SDZ";
	const std::string NumbersLeft   = StenoNumbers.substr( 0,  7);
	const std::string NumbersMiddle = StenoNumbers.substr( 7,  5);
	const std::string NumbersRight  = StenoNumbers.substr(12, 10);
	// Conversion look-ups:
	const std::string Numbers    = "1234506789";
	const std::string NumbersMap = "STPHAOFPLT";

	constexpr auto npos = std::string::npos;
	auto in(std::string_view set) {
		return [set](char c) {
			return set.find(c) != npos;
		};
	};

	std::string replaceNums(std::string/*by copy*/ str) {
		// Don't forget about the dash! Information would be removed otherwise.
		if (str.find_first_of(NumbersMiddle + Dash) == npos) {
			auto i = std::min(str.find_first_of(NumbersRight), str.size());
			str.insert(i, 1, Dash);
		}
		// Iterate and replace.
		for (char& c : str) {
			if (auto i = Numbers.find(c); i != npos) c = NumbersMap[i];
		}
		return str;
	}

	auto decomposeStroke(std::string str) {
		auto i0 = str.find_first_of(Middle + Dash);
		auto i1 = str.find_last_of(Middle + Dash) + 1;
		auto left   = str.substr( 0,         i0 -  0);
		auto middle = str.substr(i0,         i1 - i0);
		auto right  = str.substr(i1, str.size() - i1);
		if (middle == std::string {Dash}) middle = "";
		return std::array {left, middle, right};
	}

	bool validOrder(std::string str, std::string Alphabet) {
		if (!std::all_of(str.begin(), str.end(), in(Alphabet))) return false;
		auto it = str.begin();
		for (char c : Alphabet) {
			if (it == str.end()) return true;
			if (*it == c) ++it;
		}
		return it == str.end();
	}

	bool validStroke(std::string str) {
		// The goal here is to have every possible valid stroke a unique string
		// representation. The user input is already normalized by Stroke's
		// constructor, so we're only making it strict for internal use.
		if (str.empty()) return false;
		// Every stroke is required to have a middle.
		if (str.find_first_of(Middle + Dash) == npos) return false;
		// Numbers will not be used internally, only '#'.
		if (str.find_first_of(Numbers) != npos) return false;
		// Check for order of components.
		const auto [left, middle, right] = decomposeStroke(str);
		if (!validOrder(left, Hash+Left)) return false;
		if (!validOrder(middle, Middle)) return false;
		if (!validOrder(right, Right)) return false;
		return true;
	}
}

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(std::string str) {
	auto strHas = [&str](auto x) { return str.find_first_of(x) != npos; };
	auto implicit = [&str](auto x) { return validStroke(x) && (str=x, true); };
	// Normalize.
	std::erase_if(str, in(Whitespace));
	if (!validStroke(str)) {
		// TODO: Decide with certainty how to handle empty input.
		if (str.empty()) str = Dash;
		// Marks cannot combine with keys. If you've seen anyone do this please let me know.
		if (strHas(Mark)) { set(Key::Mark); return; }
		// Remove other flags.
		if (str.front() == Tilde) str = {++str.begin(), str.end()}, set(Key::OpenLeft );
		if (str.back()  == Tilde) str = {str.begin(), --str.end()}, set(Key::OpenRight);
		// Accept an implicit dash.
		if (implicit(str + Dash)); else
		// Numbers are replaced by letters + Num flag.
		if (strHas(Numbers) && validOrder(str, Hash + StenoNumbers)) {
			if (!strHas(Hash) && implicit(Hash + replaceNums(str))); else
			if ( strHas(Hash) && implicit(       replaceNums(str))); else
			{ failConstruction(); return; }
		} else
		// Unable to normalize.
		if (!validStroke(str)) { failConstruction(); return; }
	}
	// Explode into 3 substrings (each w/ unique elements).
	const auto [left, middle, right] = decomposeStroke(str);
	// Assign.
	for (char c : left  ) bits[Left  .find(c) +  1] = true;
	for (char c : middle) bits[Middle.find(c) +  8] = true;
	for (char c : right ) bits[Right .find(c) + 13] = true;
}

Stroke::Stroke(std::span<const char> s) {
	*this = Stroke(std::string {s.begin(), s.end()});
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

void Stroke::failConstruction() { this->keys.FailedConstruction = true; }

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

Strokes::Strokes(std::string s) {
	auto push = [&](unsigned i, unsigned j) {
		// if (i == j) return false;
		this->list.emplace_back(s.substr(i, j-i));
		if (this->list.back().keys.FailedConstruction) return false;
		return true;
	};

	signed i=0, j=0;
	while (j=s.find('/', i), j!=npos) {
		if (push(i, j) == false) return;
		i = j+1;
	}
	push(i, s.size());
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

Brief::Brief(std::string str, Strokes xx): strokes{xx}, text{str} { normalize(); }
Brief::Brief(std::string str, Brief b): strokes{b.strokes}, text{str} { normalize(); }

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

Brief operator+ (Brief  b , Modifier f) { return f(b); }
Brief operator| (Brief  b , Modifier f) { return f(b); }
Brief operator+=(Brief& b , Modifier f) { return b = f(b); }
Brief operator|=(Brief& b , Modifier f) { return b = f(b); }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string toString(Stroke x) {
	if (x.keys.OpenLeft) {
		x.keys.OpenLeft = false;
		auto result = '~' + toString(x);
		auto i = result.find(' ');
		if (i != npos) result.erase(i, 1);
		return result;
	}

	if (x.keys.OpenRight) {
		x.keys.OpenRight = false;
		auto result = toString(x) + '~';
		auto i = result.rfind(' ');
		if (i != npos) result.erase(i, 1);
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
