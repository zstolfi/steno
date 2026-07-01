#include "steno.hh"

namespace steno {

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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

Stroke& Stroke::clear() {
	*this = NoStroke;
	return *this;
}

// Range-for compatibility
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

// Fail-state query
Issues<Key> Stroke::issues() const {
	if (m_bits & FailBit) return {Key(FailBit)};
//	else return NoIssue;
	else return {};
}

Stroke::operator bool() const {
	return *this != NoStroke && !issues();
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
	return *this = bool(r);
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
	auto old = *this;
	++(*this);
	return old;
}

Key Stroke::Iterator::operator*() const {
	assert(m_bits != 0);
	return Key(std::bit_floor(m_bits));
}

// Internal
uint32_t Stroke::getFlags() const {
	return m_bits & FlagsMask;
}

void Stroke::setFlags(uint32_t flags) {
	m_bits &= ~FlagsMask;
	m_bits |= flags;
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

/* ~~ Chain Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Chain::Chain(std::string_view str) {
	// How to spell the empty chain (\s*-\s*)
	if (auto i = str.find_first_not_of(" \t"); i != str.npos)
	if (auto j = str.find_last_not_of(" \t"); j != str.npos)
	if (i == j && str.find('-') != str.npos) return;
	// Split up strokes by "/" otherwise
	auto push = [&](auto i, auto j) { emplace_back(str.substr(i, j-i)); };
	int i=0, j=0;
	while (j=str.find('/', i), j!=str.npos) {
		push(i, j);
		i = j+1;
	}
	push(i, str.size());
}

Chain::Chain(Stroke x) {
	insert(end(), x);
}

Chain::Chain(std::span<Stroke const> span) {
	insert(end(), span.begin(), span.end());
}

// Concatenation
Chain& Chain::operator|=(Chain p) {
	insert(end(), p.begin(), p.end());
	return *this;
}

Chain operator|(Chain lhs, Chain const& rhs) {
	lhs |= rhs; return lhs;
}

// Stroke promotion
Chain operator|(Stroke lhs, Stroke const& rhs) {
	return Chain {lhs, rhs};
}

// Fail-state query
Issues<Stroke const*> Chain::issues() const {
//	if (empty()) return NoIssues;
	if (empty()) return {};

	Issues<Stroke const*> result {};
	for (Stroke const& s : *this) {
		if (s == NoStroke || s.issues()) result.push_back(&s);
	}
	return result;
}

Chain::operator bool() const {
	if (empty()) return false;

	for (Stroke const& s : *this) {
		if (s == NoStroke || s.issues()) return false;
	}
	return true;
}

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Brief::Brief(Chain const& p, Phrase s)
: m_chain{p}, m_phrase{s} { normalize(); }

Brief::Brief(Brief const& b, Phrase s)
: m_chain{b.m_chain}, m_phrase{s} { normalize(); }

// Getters and Setters
Chain& Brief::chain() {
	return m_chain;
}

Chain const& Brief::chain() const {
	return m_chain;
}

Phrase& Brief::phrase() {
	return m_phrase;
}

Phrase const& Brief::phrase() const {
	return m_phrase;
}

Brief& Brief::clear() {
	*this = NoBrief;
	return *this;
}

// Fail-state query
Issues<Stroke const*> Brief::issues() const {
	return m_chain.issues();
}

Brief::operator bool() const {
	if (m_chain.empty() && m_phrase.empty()) return false;
	else return (bool)m_chain;
}

// Concatenation
Brief& Brief::operator|=(Brief other) {
	m_chain |= other.m_chain;
	m_phrase += other.m_phrase;
	return *this;
}

Brief operator|(Brief lhs, Brief const& rhs) {
	lhs |= rhs; return lhs;
}

Brief& Brief::operator+=(Phrase str) {
	m_phrase += str;
	return *this;
}

Brief operator+(Brief b, Phrase str) {
	b += str; return b;
}

Brief operator+(Phrase str, Brief b) {
	Brief result {b.m_chain, str};
	return result += str;
}

// Internal
Brief& Brief::normalize() {
	// Remove empty strokes in m_chain.
	m_chain.erase(
		std::remove(m_chain.begin(), m_chain.end(), NoStroke),
		m_chain.end()
	);
	// Remove leading or trailing whitespace.
	constexpr std::string_view Whitespace {" \t\n\r"};
	auto i = m_phrase.find_first_not_of(Whitespace);
	auto j = m_phrase.find_last_not_of(Whitespace);
	m_phrase = (i != m_phrase.npos)? m_phrase.substr(i, j-i + 1): "";
	return *this;
}

// Chain promotion
Brief operator+(Chain p, Phrase str) {
	return Brief {p, ""} + str;
}

Brief operator+(Phrase str, Chain p) {
	return str + Brief {p, ""};
}

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace /*detail*/ {
	constexpr struct {
		bool operator()(Brief const& a, Brief const& b) const {
			return a.chain() < b.chain();
		}
	} EntryCompare {};
}

Dictionary::Dictionary(std::span<Brief const> span) {
	insert(span.begin(), span.end());
}

void Dictionary::clean() {
	std::erase_if(m_entries, [] (Brief const& b) {
		return b.issues()
		||     b.chain() == NoChain
		||     b.phrase() == NoPhrase;
	});
}

// TODO: Optional argument for how to handle insertion,
//       or maybe have the comparator decide what to do.
Dictionary::iterator Dictionary::insert(Brief const& b) {
	// Efficiently find our sorted position
	auto position = std::lower_bound(begin(), end(), b, EntryCompare);
	// Our entry doesn't already exist
	if (position == end() || position->chain() != b.chain()) {
		// Insert our entry sorted
		return m_entries.insert(position, b);
	}
	// Entry already exists
	else {
		// Overwrite previous value
		position->phrase() = b.phrase();
		return position;
	}
}

void Dictionary::insert(std::initializer_list<Brief> il) {
	insert(il.begin(), il.end());
}

std::size_t Dictionary::erase(Chain p) {
	auto it = find(p);
	if (it != end()) m_entries.erase(it);
	return it != end();
}

Dictionary::iterator Dictionary::erase(const_iterator it) {
	return m_entries.erase(it);
}

Dictionary::iterator Dictionary::erase(const_iterator i, const_iterator j) {
	return m_entries.erase(i, j);
}

void Dictionary::merge(Dictionary& other) {
	for (Brief& b : other) insert(b);
}

void Dictionary::merge(Dictionary&& other) {
	for(Brief& b : std::move(other)) insert(b);
}

void Dictionary::clear() {
	m_entries.clear();
}

bool Dictionary::contains(Chain const& p) const {
	return find(p) != end();
}

Dictionary::iterator Dictionary::find(Chain const& p) {
	auto it = std::lower_bound(begin(), end(), Brief {p, ""}, EntryCompare);
	if (it == end() || it->chain() != p) return end();
	else return it;
}

Dictionary::const_iterator Dictionary::find(Chain const& p) const {
	auto it = std::lower_bound(begin(), end(), Brief {p, ""}, EntryCompare);
	if (it == end() || it->chain() != p) return end();
	else return it;
}

Dictionary::iterator Dictionary::lower_bound(Chain const& p) {
	return find(p);
}

Dictionary::const_iterator Dictionary::lower_bound(Chain const& p) const {
	return find(p);
}

Dictionary::iterator Dictionary::upper_bound(Chain const& p) {
	auto it = find(p);
	if (it != end()) return it+1;
	else return end();
}

Dictionary::const_iterator Dictionary::upper_bound(Chain const& p) const {
	auto it = find(p);
	if (it != end()) return it+1;
	else return end();
}

std::pair<Dictionary::iterator, Dictionary::iterator>
Dictionary::equal_range(Chain const& p) {
	auto it = find(p);
	if (it != end()) return {it, it+1};
	else return {end(), end()};
}

std::pair<Dictionary::const_iterator, Dictionary::const_iterator>
Dictionary::equal_range(Chain const& p) const {
	auto it = find(p);
	if (it != end()) return {it, it+1};
	else return {end(), end()};
}

// Map methods
Phrase& Dictionary::operator[](Chain const& p) {
	auto it = find(p);
	if (it != end()) return it->phrase();
	else {
		auto it = emplace(p, NoPhrase);
		return it->phrase();
	}
}

Phrase& Dictionary::at(Chain const& p) {
	auto it = find(p);
	if (it != end()) return it->phrase();
	else throw std::out_of_range {toString(p)};
}

Phrase const& Dictionary::at(Chain const& p) const {
	auto it = find(p);
	if (it != end()) return it->phrase();
	else throw std::out_of_range {toString(p)};
}

void Dictionary::sort() {
	std::sort(begin(), end(), EntryCompare);
}

/* ~~ String Output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Format operator|(Format f, Format g) {
	return Format(long(f) | long(g));
}

Format operator|=(Format& lhs, Format rhs) {
	return lhs = lhs | rhs;
}

namespace /*detail*/ {
	constexpr int keyID(steno::Key k) {
		return 31 - std::countr_zero(uint32_t(k));
	}

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

std::string toString(Key k, Format format) {
	auto result = std::string(1, toChar(k));
	if (hyphen(format)) {
		if (k & Stroke {"STKPWHR-          "}) result = result + '-';
		if (k & Stroke {"       -FRPBLGTSDZ"}) result = '-' + result;
	}
	return result;
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

	if ( NumBar &&  (alpha  (format) || !AnyNumbers)) put(Key::Num);
	if (!Middle && !(numeric(format) &&  AllNumbers)) put(Key::x, '-');
	for (Key key : Left  ) put(key);
	for (Key key : Middle) put(key);
	for (Key key : Right ) put(key);
	if (packed(format)) std::erase(result, ' ');
	return result;
}

std::string toString(Chain const& p, Format format) {
	std::string result = "";
	for (int i=0; auto stroke : p) {
		if (i++) result += '/';
		result += toString(stroke, format);
	}
	return result;
}

std::string toString(Brief const& b, Format format) {
	return toString(b.chain(), format) + ", " + b.phrase();
}

std::ostream& operator<<(std::ostream& os, Stroke s) {
	auto format = Format(os.iword(Format_xalloc));
	if (!bits(format)) format = StrokeDefault;
	return os << toString(s, format);
}

std::ostream& operator<<(std::ostream& os, Chain const& p) {
	auto format = Format(os.iword(Format_xalloc));
	if (!bits(format)) format = StrokeDefault;
	return os << toString(p, format);
}

std::ostream& operator<<(std::ostream& os, Brief const& b) {
	auto format = Format(os.iword(Format_xalloc));
	if (!bits(format)) format = StrokeDefault;
	return os << toString(b, format);
}

// Format as manipulator
std::ostream& operator<<(std::ostream& os, Format f) {
	long& iword = os.iword(Format_xalloc);
	if (!iword) iword = bits(StrokeDefault);
	iword &= ~mask(f);
	iword |= bits(f);
	return os;
}

/* ~~ Misc. STL Functionality ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno

std::size_t std::hash<steno::Stroke>::operator()(steno::Stroke const& x) const {
	return std::hash<uint32_t> {} (x.m_bits);
}

// https://stackoverflow.com/a/72073933
std::size_t std::hash<steno::Chain>::operator()(steno::Chain const& x) const {
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
