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
		emplace_back(str.substr(i, j-i));
		if (back().failed()) return false;
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
	insert(x);
}

Phrase::Phrase(std::span<Stroke const> span) {
	insert(span.begin(), span.end());
}

// Fail-state query
bool Phrase::failed() const {
	// TODO: Decide whether containing an empty stroke is an error.
	return std::any_of(
		begin(), end(),
		[](auto x) { return x.failed(); }
	);
}

Phrase::operator bool() const {
	return !empty() && !failed();
}

// Concatenation
Phrase& Phrase::operator|=(Phrase p) {
	insert(end(), p.begin(), p.end());
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

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Dictionary::Dictionary(std::span<Stroke const> span) {
	insert(span.begin(), span.end());
}

// TODO
bool failed() const;
void eraseFailed();

// TODO: Optional argument for how to handle insertion
Dictionary::iterator Dictionary::insert(Brief b) {
	// Append to our linked list
	auto it = m_list.insert(m_list.end(), b);
	// Efficiently find our sorted position
	auto next = m_set.upper_bound(it);
	// Insert into our set
	m_set.insert(next, it);
	// Move our linked-list element to its rightful place
	if (next != m_set.end()) {
		m_list.splice(*next, m_list, it);
	}
	return it;
}

std::size_t erase(Phrase p) {
	auto it2 = m_set.find(p);
	if (it2 == m_set.end()) return 0;
	m_set.erase(it2);
	m_list.erase(*it2);
	return 1;
}

/* ~~ String Output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Format operator|(Format f, Format g) {
	return Format(long(f) | long(g));
}

Format operator|=(Format& lhs, Format rhs) {
	return lhs = lhs | rhs;
}

namespace /*detail*/ {
	int const Format_xalloc = std::ios_base::xalloc();

	constexpr long bits(Format f) { return long(f); }
	constexpr long mask(Format f) {
		if (f == Packed  ) return 0b00'00'11;
		if (f == Wide    ) return 0b00'00'11;
		if (f == Hyphen  ) return 0b00'11'00;
		if (f == NoHyphen) return 0b00'11'00;
		if (f == Numeric ) return 0b11'00'00;
		if (f == Alpha   ) return 0b11'00'00;
		return {};
	}

	template <Format F>
	bool check(Format x) { return (mask(F) & bits(x)) == bits(F); }
	[[maybe_unused]] bool packed  (Format x) { return check<Packed  >(x); }
	[[maybe_unused]] bool wide    (Format x) { return check<Wide    >(x); }
	[[maybe_unused]] bool hyphen  (Format x) { return check<Hyphen  >(x); }
	[[maybe_unused]] bool nohyphen(Format x) { return check<NoHyphen>(x); }
	[[maybe_unused]] bool numeric (Format x) { return check<Numeric >(x); }
	[[maybe_unused]] bool alpha   (Format x) { return check<Alpha   >(x); }
}

char toChar(Key k) {
	return "#STKPWHRAO*EUFRPBLGTSDZ" [keyID(k)];
}

char toCharShift(Key k) {
	// The keyboard "shifted" via the number bar.
	return "#12K3W4R50*EU6R7B8G9SDZ" [keyID(k)];
}

std::string toString(Key k) {
	return std::string(1, toChar(k));
}

std::string toString(Stroke s, Format format) {
	Stroke const NumBar = s & Stroke {"#         -            "};
	Stroke const Left   = s & Stroke {" STKPWHR  -            "};
	Stroke const Middle = s & Stroke {"        AO*EU          "};
	Stroke const Right  = s & Stroke {"          -  FRPBLGTSDZ"};
	bool const AnyNumbers = NumBar && (s & steno::Stroke{" STPHAOFPLT"});
	bool const AllNumbers = NumBar && (s & steno::Stroke{"#STPHAOFPLT"}) == s;
	auto const toCh = NumBar && numeric(format)? toCharShift: toChar;

	std::string result (Stroke::KeyCount, ' ');
	auto put = [&] (Key k, char c = '\0') { result[keyID(k)] = c? c: toCh(k); };

	if ( NumBar && !(numeric(format) || !AnyNumbers)) put(Key::Num);
	if (!Middle && !(numeric(format) &&  AllNumbers)) put(Key::x, '-');
	for (Key key : Left  ) put(key);
	for (Key key : Middle) put(key);
	for (Key key : Right ) put(key);
	if (packed(format)) std::erase(result, ' ');
	return result;
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
