#include "steno.hh"
#include <algorithm>
#include <cctype>

namespace /*detail*/ {
	constexpr auto FailKey = steno::Key(31);
	inline constexpr auto bitFromKey(steno::Key k) { return 31-int(k); }
	constexpr auto FlagsMask = 0b00000000000000000000000'111111111;
}

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(FromBits_Arg, std::bitset<23> b) {
	for (unsigned i=0; i<b.size(); i++) if (b[22-i]) this->set(Key(i));
}

Stroke::Stroke(FromBitsReversed_Arg, std::bitset<23> b) {
	for (unsigned i=0; i<b.size(); i++) if (b[i]) this->set(Key(i));
}

bool Stroke::failed() const {
	return this->get(FailKey);
}

Stroke::operator bool() const {
	return this->bits && !this->failed();
}

uint32_t Stroke::getBits() const {
	return this->bits;
}

bool Stroke::get(Key k) const {
	return this->bits >> bitFromKey(k) & 1;
}

Stroke& Stroke::set(Key k, bool b) {
	if (b == false) return this->unset(k);
	this->bits |= 1 << bitFromKey(k);
	return *this;
}

Stroke& Stroke::unset(Key k) {
	this->bits &= ~(1 << bitFromKey(k));
	return *this;
}

Stroke::Reference::operator bool() const {
	return parent->get(key);
}

Stroke::Reference& Stroke::Reference::operator=(bool b) {
	parent->set(key, b);
	return *this;
}

Stroke::Reference& Stroke::Reference::operator=(Reference const& r) {
	return *this = (bool)r;
}

bool Stroke::operator[](Key k) const {
	return this->get(k);
}

Stroke::Reference Stroke::operator[](Key k) {
	return Stroke::Reference {this, k};
}

Stroke Stroke::operator~() const {
	Stroke result {};
	result.bits = ~this->bits & ~FlagsMask;
	return result;
}

Stroke& Stroke::operator+=(Stroke other) {
	auto flags = other.getFlags();
	this->bits |= other.bits;
	this->setFlags(flags);
	return *this;
}

Stroke& Stroke::operator-=(Stroke other) {
	auto flags = other.getFlags();
	this->bits &= ~other.bits;
	this->setFlags(flags);
	return *this;
}

Stroke& Stroke::operator&=(Stroke other) {
	auto flags = other.getFlags();
	this->bits &= other.bits;
	this->setFlags(flags);
	return *this;
}

Stroke& Stroke::operator^=(Stroke other) {
	auto flags = other.getFlags();
	this->bits ^= other.bits;
	this->setFlags(flags);
	return *this;
}

uint32_t Stroke::getFlags() const {
	return this->bits & FlagsMask;
}

void Stroke::setFlags(uint32_t flags) {
	this->bits &= ~FlagsMask;
	this->bits |= flags;
}

void Stroke::failConstruction(std::string_view str) {
//	if (str != "") std::cout << str << "\n";
	this->set(FailKey);
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
		if (this->list.back().failed()) return false;
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

Stroke  operator^(Stroke  x , Stroke  y ) { return x ^= y; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string toString(Key k) {
	switch (k) {
	case Key::Num: return "#";
	case Key::S_: return "S";
	case Key::T_: return "T"; case Key::K_: return "K";
	case Key::P_: return "P"; case Key::W_: return "W";
	case Key::H_: return "H"; case Key::R_: return "R";
	case Key::A : return "A"; case Key::O : return "O";
	case Key::x : return "*";
	case Key::E : return "E"; case Key::U : return "U";
	case Key::_F: return "F"; case Key::_R: return "R";
	case Key::_P: return "P"; case Key::_B: return "B";
	case Key::_L: return "L"; case Key::_G: return "G";
	case Key::_T: return "T"; case Key::_S: return "S";
	case Key::_D: return "D"; case Key::_Z: return "Z";
	default: return {};
	}
}

std::string toString(Stroke x) {
	if (x.get(Key::OpenLeft)) {
		x.unset(Key::OpenLeft);
		auto result = '~' + toString(x);
		auto i = result.find(' ');
		if (i != result.npos) result.erase(i, 1);
		return result;
	}

	if (x.get(Key::OpenRight)) {
		x.unset(Key::OpenRight);
		auto result = toString(x) + '~';
		auto i = result.rfind(' ');
		if (i != result.npos) result.erase(i, 1);
		return result;
	}

	std::string result = "#STKPWHRAO*EUFRPBLGTSDZ ";
	for (unsigned i=0; i<result.size(); i++) {
		if (x.get(Key(i)) == false) result[i] = ' ';
	}
	if (!(x & Stroke {"AO*EU"})) {
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

std::size_t std::hash<steno::Stroke>::operator()(steno::Stroke const& x) const {
	return std::hash<decltype(x.getBits())> {}(x.getBits());
}
