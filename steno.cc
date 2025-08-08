#include "steno.hh"

namespace /*detail*/ {
	constexpr auto FailBit   = 0b00000000000000000000000'000000001;
	constexpr auto FlagsMask = 0b00000000000000000000000'111111111;
}

namespace steno {

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Fail-state query
bool Stroke::failed() const {
	return m_bits & FailBit;
}

Stroke::operator bool() const {
	return m_bits && !failed();
}

// Getters and Setters
uint32_t Stroke::bits() const {
	return m_bits;
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
	result.m_bits = ~m_bits & ~FlagsMask;
	return result;
}

Stroke& Stroke::operator+=(Stroke const& other) {
	auto flags = getFlags();
	m_bits |= other.m_bits;
	setFlags(flags);
	return *this;
}

Stroke& Stroke::operator-=(Stroke const& other) {
	auto flags = getFlags();
	m_bits &= ~other.m_bits;
	setFlags(flags);
	return *this;
}

Stroke& Stroke::operator&=(Stroke const& other) {
	auto flags = getFlags();
	m_bits &= other.m_bits;
	setFlags(flags);
	return *this;
}

Stroke& Stroke::operator^=(Stroke const& other) {
	auto flags = getFlags();
	m_bits ^= other.m_bits;
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
	return m_bits & FlagsMask;
}

void Stroke::setFlags(uint32_t flags) {
	m_bits &= ~FlagsMask;
	m_bits |= flags;
}

void Stroke::failConstruction(std::string_view str) {
//	if (str != "") std::cout << str << "\n";
	m_bits |= FailBit;
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
		m_strokes.emplace_back(str.substr(i, j-i));
		if (m_strokes.back().failed()) return false;
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
	m_strokes = std::vector<Stroke> {x};
}

Phrase::Phrase(std::span<Stroke const> span) {
	m_strokes = std::vector<Stroke> (span.begin(), span.end());
}

// Fail-state query
bool Phrase::failed() const {
	// TODO: Decide whether containing an empty stroke is an error.
	return std::any_of(
		m_strokes.begin(), m_strokes.end(),
		[](auto x) { return x.failed(); }
	);
}

Phrase::operator bool() const {
	return !m_strokes.empty() && !failed();
}

// Concatenation
Phrase& Phrase::operator|=(Phrase p) {
	m_strokes.insert(
		m_strokes.end(),
		p.m_strokes.begin(), p.m_strokes.end()
	);
	return *this;
}

Phrase operator|(Phrase lhs, Phrase const& rhs) {
	lhs |= rhs; return lhs;
}

// Stroke promotion
Phrase operator|(Stroke lhs, Stroke const& rhs) {
	return Phrase {lhs, rhs};
}

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Brief::Brief(Phrase const& p, std::string_view s)
: m_phrase{p}, m_text{s} { normalize(); }

Brief::Brief(Brief const& b, std::string_view s)
: m_phrase{b.m_phrase}, m_text{s} { normalize(); }

// Fail-state query
bool Brief::failed() const {
	return std::any_of(
		m_phrase.begin(), m_phrase.end(),
		[](auto s) { return !s; }
	);
}

Brief::operator bool() const {
	return !m_phrase.empty() && !failed();
}

// Getters and Setters
Phrase& Brief::phrase() {
	return m_phrase;
}

Phrase const& Brief::phrase() const {
	return m_phrase;
}

std::string& Brief::text() {
	return m_text;
}

std::string const& Brief::text() const {
	return m_text;
}

// Concatenation
Brief& Brief::operator|=(Brief other) {
	m_phrase |= other.m_phrase;
	m_text += other.m_text;
	return *this;
}

Brief operator|(Brief lhs, Brief const& rhs) {
	lhs |= rhs; return lhs;
}

Brief& Brief::operator+=(std::string_view str) {
	m_text += str;
	return *this;
}

Brief operator+(Brief b, std::string_view str) {
	b += str; return b;
}

Brief operator+(std::string_view str, Brief b) {
	Brief result {b.m_phrase, str};
	return result += str;
}


// Internal

Brief& Brief::normalize() {
	// Remove empty m_phrase.
	m_phrase.erase(
		std::remove(m_phrase.begin(), m_phrase.end(), NoStroke),
		m_phrase.end()
	);
	// Remove leading or trailing whitespace.
	constexpr std::string_view Whitespace {" \t\n\r"};
	auto i = m_text.find_first_not_of(Whitespace);
	auto j = m_text.find_last_not_of(Whitespace);
	m_text = (i != m_text.npos)? m_text.substr(i, j-i + 1): "";
	return *this;
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
	return std::hash<uint32_t> {} (x.m_bits);
}

// https://stackoverflow.com/a/72073933
std::size_t std::hash<steno::Phrase>::operator()(steno::Phrase const& x) const {
	std::size_t seed = x.size();
	for (auto stroke : x) {
		uint32_t n = stroke.m_bits;
		n = ((n >> 16) ^ n) & 0x45D9F3B;
		n = ((n >> 16) ^ n) & 0x45D9F3B;
		n = (n >> 16) ^ n;
		seed ^= x + 0x9E3779B9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}
