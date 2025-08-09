#include "steno.hh"

namespace /*detail*/ {
	constexpr auto FailBit   = 0b00000000000000000000000'000000001;
	constexpr auto FlagsMask = 0b00000000000000000000000'111111111;
	constexpr int keyID(steno::Key k) {
		return 31 - std::countr_zero((uint32_t)k);
	}
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
uint32_t Stroke::raw() const {
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

// Subscript operator
Stroke::Reference Stroke::operator[](Key k) {
	return Stroke::Reference {this, k};
}

bool Stroke::operator[](Key k) const {
	return get(k);
}

// Range-for compatability
Stroke::Iterator Stroke::begin() const {
	return Iterator {*this};
}

Stroke::Iterator Stroke::end() const {
	return Iterator {};
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

// Key proxy classes
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

bool Stroke::Iterator::operator==(Iterator const& other) const {
	return std::bit_floor(this->m_bits)
	==     std::bit_floor(other.m_bits);
}

Stroke::Iterator& Stroke::Iterator::operator++() {
	m_bits &= ~std::bit_floor(m_bits); // Remove leading bit.
	return *this;
}

Stroke::Iterator Stroke::Iterator::operator++(int) {
	auto result = *this;
	++(*this);
	return result;
}

Key Stroke::Iterator::operator*() const {
	// Invalid if bit == 0
	return (Key)std::bit_floor(m_bits);
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
//	// Remove leading or trailing whitespace.
//	constexpr std::string_view Whitespace {" \t\n\r"};
//	auto i = m_text.find_first_not_of(Whitespace);
//	auto j = m_text.find_last_not_of(Whitespace);
//	m_text = (i != m_text.npos)? m_text.substr(i, j-i + 1): "";
	return *this;
}

// Phrase promotion
Brief operator+(Phrase p, std::string_view str) {
	return Brief {p, ""} + str;
}

Brief operator+(std::string_view str, Phrase p) {
	return str + Brief {p, ""};
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~ String Output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Format operator|(Format f, Format g) {
	return Format(long(f) | long(g));
}

namespace /*detail*/ {
	int const Format_xalloc = std::ios_base::xalloc();

	long bits(Format f) {
		return long(f);
	}

	long mask(Format f) {
		if (f == Packed  ) return 0b00'00'11;
		if (f == Wide    ) return 0b00'00'11;
		if (f == Hyphen  ) return 0b00'11'00;
		if (f == NoHyphen) return 0b00'11'00;
		if (f == Numeric ) return 0b11'00'00;
		if (f == Alpha   ) return 0b11'00'00;
		return {};
	}

	bool has(Format f, Format stock) {
		return (mask(stock) & bits(f)) == bits(stock);
	} 
}

char toChar(Key k) {
	switch (k) {
	case Key::Num: return '#';
	case Key::S_: return 'S';
	case Key::T_: return 'T'; case Key::K_: return 'K';
	case Key::P_: return 'P'; case Key::W_: return 'W';
	case Key::H_: return 'H'; case Key::R_: return 'R';
	case Key::A : return 'A'; case Key::O : return 'O';
	case Key::x : return '*';
	case Key::E : return 'E'; case Key::U : return 'U';
	case Key::_F: return 'F'; case Key::_R: return 'R';
	case Key::_P: return 'P'; case Key::_B: return 'B';
	case Key::_L: return 'L'; case Key::_G: return 'G';
	case Key::_T: return 'T'; case Key::_S: return 'S';
	case Key::_D: return 'D'; case Key::_Z: return 'Z';
	default: return {};
	}
}

std::string toString(Key k) {
	return std::string(1, toChar(k));
}

std::string toString(Stroke s, Format format) {
	Stroke const Left   = s & Stroke {"#STKPWHR  -            "};
	Stroke const Middle = s & Stroke {"        AO*EU          "};
	Stroke const Right  = s & Stroke {"          -  FRPBLGTSDZ"};
	if (has(format, Format::Packed)) {
		std::string result {};
		for (Key key : Left  ) result += toChar(key);
		/* TODO */
		if (!Middle) result += '-';
		for (Key key : Middle) result += toChar(key);
		for (Key key : Right ) result += toChar(key);
		return result;
	}
	else if (has(format, Format::Wide)) {
		std::string result (Stroke::KeyCount, ' ');
		for (Key key : s) result[keyID(key)] = toChar(key);
		if (!Middle) result[keyID(Key::x)] = '-';
		return result;
	}
	else return {};
}

std::string toString(Phrase p, Format format) {
	std::string result = "";
	for (int i=0; auto stroke : p) {
		if (i++) result += '/';
		result += toString(stroke, format);
	}
	return result;
}

std::ostream& operator<<(std::ostream& os, Stroke s) {
	auto format = Format(os.iword(Format_xalloc));
	return os << toString(s, format);
}

std::ostream& operator<<(std::ostream& os, Phrase p) {
	auto format = Format(os.iword(Format_xalloc));
	return os << toString(p, format);
}

// Format as manipulator
std::ostream& operator<<(std::ostream& os, Format f) {
	os.iword(Format_xalloc) &= ~mask(f);
	os.iword(Format_xalloc) |= bits(f);
	return os;
}

/* ~~ Misc. STL Functionality ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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
