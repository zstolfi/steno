#include "steno.hh"
#include <algorithm>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(std::string s) {
	const std::string Whitespace = " \t\r\n";
	const std::string Left   = "STKPWHR";
	const std::string Middle = "AO*EU";
	const std::string Right  = "FRPBLGTSDZ";
	const std::string Numbers    = "0123456789";
	const std::string NumbersMap = "OSTPHAFPLT";
	const char Num   = '#';
	const char Dash  = '-';
	const char Tilde = '~';
	const std::string  Other = {Num, Dash, Tilde};
	auto in = [](std::string set) {
		return [set](char c) { return set.find(c) != set.npos; };
	};

	// Remove whitespace.
	s.erase(
		std::remove_if(s.begin(), s.end(), in(Whitespace)),
		s.end()
	);

	// Handle flags.
	if (s.front() == Tilde) s=s.substr(1), this->keys.OpenLeft  = true;
	if (s.back () == Tilde) s.pop_back() , this->keys.OpenRight = true;
	if (s.front() == Num  ) s=s.substr(1), this->keys.Num       = true;

	// Normalize number keys.
	if (std::any_of(s.begin(), s.end(), in(Numbers))) {
		this->keys.Num = true;
		for (char& c : s) if (in(Numbers)(c)) c = NumbersMap[c-'0'];
	}

	// Sinple charset test:
	if (!std::all_of(s.begin(), s.end(), in(Left + Middle + Right + Other))) {
		failConstruction(); return;
	}

	// Find unique middle secion (vowels).
	unsigned v0 {}, v1 {};
	if (auto d = s.find(Dash); d != s.npos) {
		v0 = d;
		v1 = d+1;
	}
	else if (auto i = s.find_first_of(Middle); i != s.npos) {
		auto j = s.find_first_not_of(Middle, i);
		v0 = i;
		v1 = (j != s.npos) ? j : s.size();
	}
	// If there is not a middle section it might be a left-hand only shortcut.
	else if (Stroke x {s + Dash}) { *this = x; return; }
	else { failConstruction(); return; }

	// Split 's' into substrings.
	std::string sL = s.substr(0, v0);
	std::string sM = s.substr(v0, v1-v0);
	std::string sR = s.substr(v1, s.size()-v1);
	if (sM == "-") sM = "";

	// More checks.
	if (!std::all_of(sL.begin(), sL.end(), in(Left  ))
	||  !std::all_of(sM.begin(), sM.end(), in(Middle))
	||  !std::all_of(sR.begin(), sR.end(), in(Right ))) {
		failConstruction(); return;
	}

	// Assign.
	for (char c : sL) this->bits[Left  .find(c) +  1] = true;
	for (char c : sM) this->bits[Middle.find(c) +  8] = true;
	for (char c : sR) this->bits[Right .find(c) + 13] = true;
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

Stroke::operator bool() const {
	return !this->keys.FailedConstruction && this->bits.to_ulong();
}

Stroke Stroke::operator+=(Stroke other) {
	this->bits |= other.bits;
	return *this;
}

Stroke Stroke::operator-=(Stroke other) {
	// Disallow deletion of the fail flag.
	other.keys.FailedConstruction = false;
	this->bits &= ~other.bits;
	return *this;
}

Stroke Stroke::operator&=(Stroke other) {
	// Disallow deletion of the fail flag.
	other.keys.FailedConstruction = true;
	this->bits &= other.bits;
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
	while (j=s.find('/', i), j!=s.npos) {
		if (push(i, j) == false) return;
		i = j+1;
	}
	push(i, s.size());
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

Brief::Brief(Strokes xx, std::string str): strokes{xx}, text{str} { normalize(); }
Brief::Brief(Brief b, std::string str): strokes{b.strokes}, text{str} { normalize(); }

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
Brief   operator+(Strokes xx, Brief   b ) { return Brief {xx, ""} += b; }
Brief   operator+(Brief   b , Strokes xx) { return b += Brief {xx, ""}; }
Brief   operator+(std::string s, Brief b) {
	if (s=="~") { b.text = s + b.text; return b; }
	else return Brief {{"-"}, s} += b;
}
Brief   operator+(Brief b, std::string s) {
	if (s=="~") { b.text = b.text + s; return b; }
	else return b += Brief {{"-"}, s};
}

Stroke  operator-(Stroke  x , Stroke  y ) { return x -= y; }
Strokes operator-(Strokes xx, Stroke  y ) { return xx.list.back() -= y; }

Strokes operator|(Stroke  x , Stroke  y ) { return Strokes {x, y}; }
Strokes operator|(Strokes xx, Stroke  y ) { return xx.append(y); }
Strokes operator|(Stroke  x , Strokes yy) { return yy.prepend(x); }
Strokes operator|(Strokes xx, Strokes yy) { return xx.append(yy); }
Brief   operator|(Brief   a , Brief   b ) { return a |= b; }
Brief   operator|(Strokes xx, Brief   b ) { return Brief {xx, ""} |= b; }
Brief   operator|(Brief b   , Strokes xx) { return b |= Brief {xx, ""}; }

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
