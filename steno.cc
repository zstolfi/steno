#include "steno.hh"

namespace steno {

namespace /*detail*/ {

template <class To, class From>
Issues<To> issuesCast(Issues<From> const& issues) {
	Issues<To> result {};
	for (From f : issues) { result.push_back(const_cast<To>(f)); }
	return result;
}

} // namespace /*detail*/

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Fail-state query
Issues<Stroke::Iterator> Stroke::issues() const {
	if (m_bits & FailBit) {
		Iterator failOnly {};
		failOnly.m_bits = FailBit;
		Issues<Iterator> result {};
		result.push_back(failOnly);
		return result;
	}
//	else return NoIssue;
	else return {};
}

Issues<Stroke::Iterator> Stroke::issues() {
	return std::as_const(*this).issues();
}

Stroke::operator bool() const {
	return *this != NoStroke && !issues();
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

Stroke& Stroke::clear() {
	*this = NoStroke;
	return *this;
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

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Phrase::Phrase(std::string_view str) {
	// How to spell the empty phrase (\s*-\s*)
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

Phrase::Phrase(Stroke x) {
	insert(end(), x);
}

Phrase::Phrase(std::span<Stroke const> span) {
	insert(end(), span.begin(), span.end());
}

// Fail-state query
Issues<Stroke const*> Phrase::issues() const {
//	if (empty()) return NoIssues;
	if (empty()) return {};

	Issues<Stroke const*> result {};
	for (Stroke const& s : m_strokes) {
		if (s == NoStroke || s.issues()) result.push_back(&s);
	}
	return result;
}

Issues<Stroke*> Phrase::issues() {
	return issuesCast<Stroke*>(
		std::as_const(*this).issues()
	);
}

Phrase::operator bool() const {
	if (empty()) return false;

	for (Stroke const& s : m_strokes) {
		if (s == NoStroke || s.issues()) return false;
	}
	return true;
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
Issues<Stroke const*> Brief::issues() const {
	return m_phrase.issues();
}

Issues<Stroke*> Brief::issues() {
	return m_phrase.issues();
}

Brief::operator bool() const {
	if (m_phrase.empty() && m_text.empty()) return false;
	else return (bool)m_phrase;
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

Brief& Brief::clear() {
	*this = NoBrief;
	return *this;
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
	// Remove empty strokes in m_phrase.
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

// Phrase promotion
Brief operator+(Phrase p, std::string_view str) {
	return Brief {p, ""} + str;
}

Brief operator+(std::string_view str, Phrase p) {
	return str + Brief {p, ""};
}

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace /*detail*/ {
	constexpr struct {
		bool operator()(Brief const& a, Brief const& b) const {
			return a.phrase() < b.phrase();
		}
	} EntryCompare {};
}

Dictionary::Dictionary(std::span<Brief const> span) {
	insert(span.begin(), span.end());
}

Issues<Brief const*> Dictionary::issues() const {
	Issues<Brief const*> result {};
	for (Brief const& b : m_entries) {
		if (b.issues()) result.push_back(&b);
	}
	return result;
}

Issues<Brief*> Dictionary::issues() {
	return issuesCast<Brief*>(
		std::as_const(*this).issues()
	);
}

void Dictionary::clean() {
	std::erase_if(m_entries, [] (Brief const& b) {
		return b.issues()
		||     b.phrase() == NoPhrase
		||     b.text() == NoText;
	});
}

// TODO: Optional argument for how to handle insertion,
//       or maybe have the comparator decide what to do.
Dictionary::iterator Dictionary::insert(Brief const& b) {
	// Efficiently find our sorted position
	auto position = std::lower_bound(begin(), end(), b, EntryCompare);
	// Our entry doesn't already exist
	if (position == end() || position->phrase() != b.phrase()) {
		// Insert our entry sorted
		return m_entries.insert(position, b);
	}
	// Entry already exists
	else {
		// Overwrite previous value
		position->text() = b.text();
		return position;
	}
}

void Dictionary::insert(std::initializer_list<Brief> il) {
	insert(il.begin(), il.end());
}

std::size_t Dictionary::erase(Phrase p) {
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

bool Dictionary::contains(Phrase const& p) const {
	return find(p) != end();
}

Dictionary::iterator Dictionary::find(Phrase const& p) {
	auto it = std::lower_bound(begin(), end(), Brief {p, ""}, EntryCompare);
	if (it == end() || it->phrase() != p) return end();
	else return it;
}

Dictionary::const_iterator Dictionary::find(Phrase const& p) const {
	auto it = std::lower_bound(begin(), end(), Brief {p, ""}, EntryCompare);
	if (it == end() || it->phrase() != p) return end();
	else return it;
}

Dictionary::iterator Dictionary::lower_bound(Phrase const& p) {
	return find(p);
}

Dictionary::const_iterator Dictionary::lower_bound(Phrase const& p) const {
	return find(p);
}

Dictionary::iterator Dictionary::upper_bound(Phrase const& p) {
	auto it = find(p);
	if (it != end()) return it+1;
	else return end();
}

Dictionary::const_iterator Dictionary::upper_bound(Phrase const& p) const {
	auto it = find(p);
	if (it != end()) return it+1;
	else return end();
}

std::pair<Dictionary::iterator, Dictionary::iterator>
Dictionary::equal_range(Phrase const& p) {
	auto it = find(p);
	if (it != end()) return {it, it+1};
	else return {end(), end()};
}

std::pair<Dictionary::const_iterator, Dictionary::const_iterator>
Dictionary::equal_range(Phrase const& p) const {
	auto it = find(p);
	if (it != end()) return {it, it+1};
	else return {end(), end()};
}

// Map methods
Text& Dictionary::operator[](Phrase const& p) {
	auto it = find(p);
	if (it != end()) return it->text();
	else {
		auto it = emplace(p, NoText);
		return it->text();
	}
}

Text& Dictionary::at(Phrase const& p) {
	auto it = find(p);
	if (it != end()) return it->text();
	else throw std::out_of_range {toString(p)};
}

Text const& Dictionary::at(Phrase const& p) const {
	auto it = find(p);
	if (it != end()) return it->text();
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

std::string toString(Phrase const& p, Format format) {
	std::string result = "";
	for (int i=0; auto stroke : p) {
		if (i++) result += '/';
		result += toString(stroke, format);
	}
	return result;
}

std::string toString(Brief const& b, Format format) {
	return toString(b.phrase(), format) + ", " + b.text();
}

std::ostream& operator<<(std::ostream& os, Stroke s) {
	auto format = Format(os.iword(Format_xalloc));
	if (!bits(format)) format = StrokeDefault;
	return os << toString(s, format);
}

std::ostream& operator<<(std::ostream& os, Phrase const& p) {
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
