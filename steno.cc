#include "steno.hh"
#include <algorithm>
#include <string_view>

namespace /*detail*/ {
	const std::string Whitespace = " \t\r\n";
	const char Hash = '#', Dash = '-', Mark = '!', Tilde = '~';
	
	const std::string Left   = "STKPWHR";
	const std::string Middle = "AO*EU";
	const std::string Right  = "FRPBLGTSDZ";

	const std::string Numbers    = "1234506789";
	const std::string NumbersMap = "STPHAOFPLT";

	const std::string Other = {Hash, Dash, Tilde};
	const std::string Body = Left + Middle + Right;
	const std::string NumbersLeft   = Numbers.substr(0, 4);
	const std::string NumbersMiddle = Numbers.substr(4, 2);
	const std::string NumbersRight  = Numbers.substr(6, 4);

	constexpr auto npos = std::string::npos;
	auto in(std::string_view set) {
		return [set](char c) {
			return set.find(c) != npos;
		};
	};

	std::string replaceNums(std::string/*by copy*/ str) {
		// Don't forget about the dash! Information would be removed otherwise.
		if (str.find(NumbersMiddle + Middle) == npos) {
			auto i = str.find_first_of(NumbersRight);
			str.insert(i, 1, Dash);
		}
		// Iterate and replace.
		for (char& c : str) if (in(Numbers)(c)) {
			c = NumbersMap[Numbers.find(c)];
		}
		return str;
	}

	bool validStroke(std::string str) {
		// Trust the user :D
		return true;
	}
}

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(std::string str) {
	auto strHas = [&str](auto x) { return str.find_first_of(x) != npos; };
	auto implicit = [&str](auto x) { return validStroke(x) && (str=x, true); };
	auto subString = [&str](auto i, auto j) { return str.substr(i, j-i); };
	// Normalize.
	std::erase_if(str, in(Whitespace));
	bool temporaryCheck = strHas(Numbers);
	if (temporaryCheck || !validStroke(str)) {
		// TODO: Decide with certainty how to handle empty input.
		if (str.empty()) str = Dash;
		// Remove flags.
		if (str.front() == Tilde) str = {++str.begin(), str.end()}, set(Key::OpenLeft );
		if (str.back()  == Tilde) str = {str.begin(), --str.end()}, set(Key::OpenRight);
		// Accept an implicit dash.
		if (implicit(str + Dash)); else
		// Numbers are not allowed to have dashes.
		if (strHas(Numbers)) {
			if (!strHas(Hash) && implicit(Hash + replaceNums(str))); else
			if ( strHas(Hash) && implicit(       replaceNums(str))); else
			{ failConstruction(); return; }
		} else
		// Unable to normalize.
		if (!validStroke(str)) { failConstruction(); return; }
	}
	// Explode into 3 substrings (each w/ unique elements).
	auto i0 = str.find_first_of(Middle+Dash);
	auto i1 = str.find_last_of(Middle+Dash)+1;
	const std::string left   = subString( 0, i0);
	const std::string middle = subString(i0, str[i0] == Dash? i0: i1);
	const std::string right  = subString(i1, str.size());
	// Assign.
	for (char c : left  ) bits[Left  .find(c) +  1] = true;
	for (char c : middle) bits[Middle.find(c) +  8] = true;
	for (char c : right ) bits[Right .find(c) + 13] = true;
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
