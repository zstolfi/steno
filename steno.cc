#include "steno.hh"

namespace /*detail*/ {
	constexpr auto FailBit   = 0b00000000000000000000000'000000001;
	constexpr auto FlagsMask = 0b00000000000000000000000'111111111;
}

namespace steno {

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Fail-state query
bool Stroke::failed() const {
	return bits & FailBit;
}

Stroke::operator bool() const {
	return bits && !failed();
}

// Getters and Setters
uint32_t Stroke::getBits() const {
	return bits;
}

bool Stroke::get(Key k) const {
	return *this & Stroke {k};
}

Stroke& Stroke::set(Key k, bool b) {
	if (!b) return unset(k);
	return *this += Stroke {k};
}

Stroke& Stroke::unset(Key k) {
	return *this -= Stroke {k};
}

// Key proxy class
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

// Subscript operator
bool Stroke::operator[](Key k) const {
	return get(k);
}

Stroke::Reference Stroke::operator[](Key k) {
	return Stroke::Reference {this, k};
}

// Key manipulation
Stroke Stroke::operator~() const {
	Stroke result {};
	result.bits = ~bits & ~FlagsMask;
	return result;
}

Stroke& Stroke::operator+=(Stroke const& other) {
	auto flags = getFlags();
	bits |= other.bits;
	setFlags(flags);
	return *this;
}

Stroke& Stroke::operator-=(Stroke const& other) {
	auto flags = getFlags();
	bits &= ~other.bits;
	setFlags(flags);
	return *this;
}

Stroke& Stroke::operator&=(Stroke const& other) {
	auto flags = getFlags();
	bits &= other.bits;
	setFlags(flags);
	return *this;
}

Stroke& Stroke::operator^=(Stroke const& other) {
	auto flags = getFlags();
	bits ^= other.bits;
	setFlags(flags);
	return *this;
}

Stroke operator+(Stroke lhs, Stroke const& rhs) {
	lhs += rhs; return lhs;
}

Stroke operator-(Stroke lhs, Stroke const& rhs) {
	lhs -= rhs; return lhs;
}

Stroke operator&(Stroke lhs, Stroke const& rhs) {
	lhs &= rhs; return lhs;
}

Stroke operator^(Stroke lhs, Stroke const& rhs) {
	lhs ^= rhs; return lhs;
}

// Internal
uint32_t Stroke::getFlags() const {
	return bits & FlagsMask;
}

void Stroke::setFlags(uint32_t flags) {
	bits &= ~FlagsMask;
	bits |= flags;
}

void Stroke::failConstruction(std::string_view str) {
//	if (str != "") std::cout << str << "\n";
	bits |= FailBit;
}

// Key promotion
Stroke operator~(Key k) {
	return ~Stroke {k};
}

Stroke operator+(Key lhs, Key rhs) {
	return Stroke {lhs} + Stroke {rhs};
}

Stroke operator-(Key lhs, Key rhs) {
	return Stroke {lhs} - Stroke {rhs};
}

Stroke operator&(Key lhs, Key rhs) {
	return Stroke {lhs} & Stroke {rhs};
}

Stroke operator^(Key lhs, Key rhs) {
	return Stroke {lhs} ^ Stroke {rhs};
}

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Phrase::Phrase(std::string_view str) {
	auto push = [&](unsigned i, unsigned j) {
		// if (i == j) return false;
		strokes.emplace_back(str.substr(i, j-i));
		if (strokes.back().failed()) return false;
		return true;
	};

	signed i=0, j=0;
	while (j=str.find('/', i), j!=str.npos) {
		if (push(i, j) == false) return;
		i = j+1;
	}
	push(i, str.size());
}

Phrase::Phrase(Stroke x) {
	strokes = std::vector<Stroke> {x};
}

Phrase::Phrase(std::span<Stroke const> span) {
	strokes = std::vector<Stroke> (span.begin(), span.end());
}

// Fail-state query
bool Phrase::failed() const {
	// TODO: Decide whether containing an empty stroke is an error.
	return std::any_of(
		strokes.begin(), strokes.end(),
		[](auto x) { return x.failed(); }
	);
}

Phrase::operator bool() const {
	return !strokes.empty() && !failed();
}

// Getters and Setters
std::vector<Stroke>& Phrase::getStrokes() {
	return strokes;
}

std::vector<Stroke> const& Phrase::getStrokes() const {
	return strokes;
}

Phrase& Phrase::append(Phrase p) {
	strokes.insert(
		strokes.end(),
		p.strokes.begin(), p.strokes.end()
	);
	return *this;
}

Phrase& Phrase::prepend(Phrase p) {
	strokes.insert(
		strokes.begin(),
		p.strokes.begin(), p.strokes.end()
	);
	return *this;
}

// Concatenation
Phrase& Phrase::operator|=(Phrase p) {
	return append(p);
}

Phrase operator|(Phrase lhs, Phrase const& rhs) {
	lhs |= rhs; return lhs;
}

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Brief::Brief(Phrase const& p, std::string_view s)
: strokes{p}, text{s} { normalize(); }

Brief::Brief(Brief const& b, std::string_view s)
: strokes{b.strokes}, text{s} { normalize(); }

// Fail-state query
bool Brief::failed() const {
	return std::any_of(
		strokes.begin(), strokes.end(),
		[](auto s) { return !s; }
	);
}

Brief::operator bool() const {
	return !strokes.empty() && !failed();
}

// Getters and Setters
Phrase& Brief::getStrokes() {
	return strokes;
}

Phrase const& Brief::getStrokes() const {
	return strokes;
}

std::string& Brief::getText() {
	return text;
}

std::string const& Brief::getText() const {
	return text;
}

Brief& Brief::setStrokes(Phrase const& p) {
	strokes = p;
	return *this;
}

Brief& Brief::setText(std::string_view s) {
	text = s;
	return *this;
}

// Concatenation
Brief& Brief::operator|=(Brief other) {
	strokes |= other.strokes;
	appendText(other.text);
	return *this;
}

Brief operator|(Brief lhs, Brief const& rhs) {
	lhs |= rhs; return lhs;
}

// Internal
void Brief::appendText(std::string_view str) {
	if (text.empty()) { text = str; return; }
	if (str.empty()) return;

	bool endGlue = text.back() == '~';
	bool startGlue = str.front() == '~';

	if (!startGlue && !endGlue) text += ' ', text += str;
	else {
		if (endGlue) text.pop_back();
		text.insert(text.size(), str, startGlue? 1: 0);
	}
}

void Brief::normalize() {
	// Remove empty strokes.
	strokes.erase(
		std::remove(strokes.begin(), strokes.end(), NoStroke),
		strokes.end()
	);
	// Remove leading or trailing whitespace.
	text = text.substr(text.find_first_not_of(" \t\n\r"));
	text = text.substr(0, text.find_last_not_of(" \t\n\r"));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//Phrase operator+(Phrase xx, Stroke y ) { return xx.getStrokes().back() += y; }
//Phrase operator+(Stroke x , Phrase yy) { return yy.getStrokes().front() += x; }
//Brief  operator+(Brief  a , Brief  b ) { return a += b; }
//Brief  operator+(Phrase xx, Brief  b ) { return Brief {"", xx} += b; }
//Brief  operator+(Brief  b , Phrase xx) { return b += Brief {"", xx}; }
//Brief  operator+(std::string s, Brief b) {
//	if (s=="~") { b.text = s + b.text; return b; }
//	else return Brief {s, {"-"}} += b;
//}
//Brief   operator+(Brief b, std::string s) {
//	if (s=="~") { b.text = b.text + s; return b; }
//	else return b += Brief {s, {"-"}};
//}

//Phrase operator-(Phrase xx, Stroke  y) { return xx.getStrokes().back() -= y; }

Phrase operator|(Stroke x , Stroke y ) { return Phrase {x, y}; }
Phrase operator|(Phrase xx, Stroke y ) { return xx.append(y); }
Phrase operator|(Stroke x , Phrase yy) { return yy.prepend(x); }
Phrase operator|(Phrase xx, Phrase yy) { return xx.append(yy); }
//Brief  operator|(Brief  a , Brief  b ) { return a |= b; }
//Brief  operator|(Phrase xx, Brief  b ) { return Brief {"", xx} |= b; }
//Brief  operator|(Brief  b , Phrase xx) { return b |= Brief {"", xx}; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//std::string toString(Key k) {
//	switch (k) {
//	case Key::Num: return "#";
//	case Key::S_: return "S";
//	case Key::T_: return "T"; case Key::K_: return "K";
//	case Key::P_: return "P"; case Key::W_: return "W";
//	case Key::H_: return "H"; case Key::R_: return "R";
//	case Key::A : return "A"; case Key::O : return "O";
//	case Key::x : return "*";
//	case Key::E : return "E"; case Key::U : return "U";
//	case Key::_F: return "F"; case Key::_R: return "R";
//	case Key::_P: return "P"; case Key::_B: return "B";
//	case Key::_L: return "L"; case Key::_G: return "G";
//	case Key::_T: return "T"; case Key::_S: return "S";
//	case Key::_D: return "D"; case Key::_Z: return "Z";
//	default: return {};
//	}
//}

//std::string toString(Stroke x) {
//	if (x.get(Key::OpenLeft)) {
//		x.unset(Key::OpenLeft);
//		auto result = '~' + toString(x);
//		auto i = result.find(' ');
//		if (i != result.npos) result.erase(i, 1);
//		return result;
//	}

//	if (x.get(Key::OpenRight)) {
//		x.unset(Key::OpenRight);
//		auto result = toString(x) + '~';
//		auto i = result.rfind(' ');
//		if (i != result.npos) result.erase(i, 1);
//		return result;
//	}

//	std::string result = "#STKPWHRAO*EUFRPBLGTSDZ ";
//	for (unsigned i=0; i<result.size(); i++) {
//		if (x.get(Key(i)) == false) result[i] = ' ';
//	}
//	if (!(x & Stroke {"AO*EU"})) {
//		result[10] = '-';
//	}
//	return result;
//}

//std::string toString(Phrase xx) {
//	std::string result = "";
//	for (int i=0; auto stroke : xx.getStrokes()) {
//		if (i++) result += '/';
//		result += toString(stroke);
//	}
//	return result;
//}

//std::ostream& operator<<(std::ostream& os, Stroke x) {
//	return os << toString(x);
//}

//std::ostream& operator<<(std::ostream& os, Phrase xx) {
//	return os << toString(xx);
//}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno

std::size_t std::hash<steno::Stroke>::operator()(steno::Stroke const& x) const {
	return std::hash<uint32_t> {} (x.bits);
}

// https://stackoverflow.com/a/72073933
std::size_t std::hash<steno::Phrase>::operator()(steno::Phrase const& x) const {
	std::size_t seed = x.size();
	for (auto stroke : x) {
		uint32_t n = stroke.bits;
		n = ((n >> 16) ^ n) & 0x45D9F3B;
		n = ((n >> 16) ^ n) & 0x45D9F3B;
		n = (n >> 16) ^ n;
		seed ^= x + 0x9E3779B9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}
