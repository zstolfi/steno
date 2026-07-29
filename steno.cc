#include "steno.hh"
#include <sstream>

namespace steno {

namespace /* detail */ {

static constexpr bool isWhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static constexpr std::string_view asEscaped(char& c) {
	if (c == '\\') return "\\\\";
	if (c == '{') return "\\{";
	if (c == '}') return "\\}";
	return {&c, 1};
}

std::string_view trimWhitespace(std::string_view str) {
	while (!str.empty() && isWhitespace(str.front())) str.remove_prefix(1);
	while (!str.empty() && isWhitespace(str.back())) str.remove_suffix(1);
	return str;
}

std::vector<std::string_view> split(std::string_view str, char delim) {
	std::vector<std::string_view> result {};
	auto push = [&](auto i, auto j) { result.push_back(str.substr(i, j-i)); };
	int i=0, j=0;
	while (j=str.find(delim, i), j!=str.npos) {
		push(i, j);
		i = j+1;
	}
	push(i, str.size());
	return result;
}

void applyCodeSwitch(Context& context, std::string_view str) {
	/* TODO */;
}

} // namespace /* detail */

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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

/* ~~ StrokeList Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
StrokeList::StrokeList(std::string_view str) {
	// How to spell the empty stroke list (\s*-\s*)
	trimWhitespace(str);
	if (str == "-") return;
	// Split up strokes by "/" otherwise
	for (auto substr : split(str, '/')) emplace_back(substr);
}

StrokeList::StrokeList(Stroke x) {
	insert(end(), x);
}

StrokeList::StrokeList(std::span<Stroke const> span) {
	insert(end(), span.begin(), span.end());
}

// Concatenation
StrokeList& StrokeList::operator|=(StrokeList p) {
	insert(end(), p.begin(), p.end());
	return *this;
}

StrokeList operator|(StrokeList lhs, StrokeList const& rhs) {
	lhs |= rhs; return lhs;
}

// Stroke promotion
StrokeList operator|(Stroke lhs, Stroke const& rhs) {
	return StrokeList {lhs, rhs};
}

// Fail-state query
Issues<Stroke const*> StrokeList::issues() const {
//	if (empty()) return NoIssues;
	if (empty()) return {};

	Issues<Stroke const*> result {};
	for (Stroke const& s : *this) {
		if (s == NoStroke || s.issues()) result.push_back(&s);
	}
	return result;
}

StrokeList::operator bool() const {
	if (empty()) return false;

	for (Stroke const& s : *this) {
		if (s == NoStroke || s.issues()) return false;
	}
	return true;
}

/* ~~ Signal Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Signal::operator bool() const {
	return *this != NoSignal;
}

/* ~~ Token Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Word* Token::word() {
	return std::get_if<Word>(this);
}

Word const* Token::word() const {
	return std::get_if<Word>(this);
}

Signal* Token::signal() {
	return std::get_if<Signal>(this);
}

Signal const* Token::signal() const {
	return std::get_if<Signal>(this);
}

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Getters
std::string Phrase::string() const {
	return toString(*this);
}

namespace /* detail */ {

void warn( ... ) {/* TODO */}

} // namespace /* detail */

// Construction
Phrase::Phrase(std::string_view str) {
	std::string buffer {""};
	bool isSignal {false}, isEscaped {false};

	auto processBuffer = [&] {
		/**/ if (!isSignal && buffer.empty()) /**/;
		else if (!isSignal) push_back(Word {buffer});
		// Signals
		else if (buffer == "#") push_back(Signal {});
		else if (buffer == "*") push_back(Signal {Undo});
		else if (buffer == "")  push_back(Signal {Cancel});
		// Complex Signals
		else if (buffer.starts_with("@")) {
			push_back(Signal {CodeSwitch, /*.localeName*/ buffer.substr(1)});
		}
		else if (buffer.starts_with("#")) {
			if (auto split = buffer.find(':', 1); split != buffer.npos) {
				push_back(Signal {SysEx,
					/*.channel*/ buffer.substr(1, split-1),
					/*.message*/ buffer.substr(split+1),
				});
			}
			else push_back(Signal {SysEx,
				/*.channel*/ buffer.substr(1),
			});
		}
		else if (GlobalPunctuation.contains(buffer)) {
			push_back(Signal {Punctuate, /*.symbol*/ buffer});
		}
		// Alternate syntax
		else {
			std::string_view prefix {}, suffix {};
			if (buffer.starts_with("^")) prefix = "^";
			if (buffer.starts_with("&")) prefix = "&";
			if (buffer.ends_with("^")) suffix = "^";

			assert(prefix.size() + suffix.size() <= buffer.size());
			std::string_view inside {
				buffer.begin()+prefix.size(),
				buffer.end()-suffix.size(),
			};

			if (prefix == "^") push_back(Signal {Punctuate, "^"});
			if (prefix == "&") push_back(Signal {Punctuate, "&"});
			for (auto word : split(inside, ' ')) push_back(Word {word});
			if (suffix == "^") push_back(Signal {Punctuate, "^"});
			if (prefix == "&") push_back(Signal {Punctuate, "!&"});
		}
		buffer.clear();
	};

	auto it = str.begin();
	do {
		char const c {it != str.end()? *it: '\0'};

		/**/ if (c == '\0') processBuffer();
		// TODO: Account for "\n", "\t", etc.
		else if (isEscaped) buffer += c, isEscaped = false;
		else if (c == '\\') isEscaped = true;

		else if (c == '{') {
			if (isSignal) warn("unexpected '{");
			processBuffer();
			isSignal = true;
		}

		else if (c == '}') {
			if (!isSignal) warn("unexpected '}");
			processBuffer();
			isSignal = false;
		}

		else if (!isSignal && isWhitespace(c)) {
			if (!buffer.empty())
			push_back(Word {buffer});
			buffer.clear();
		}

		else buffer += c;
	}
	while (it++ != str.end());

	/**/ if (isSignal) warn("expected '}'");
	else if (isEscaped) warn("expected character after '\\'");
}

// Concatenation
Phrase& Phrase::operator+=(Phrase p) {
	for (auto token : p) {
		this->push_back(token);
	}
	return *this;
}

Phrase& Phrase::operator+=(Token t) {
	this->push_back(t);
	return *this;
}

Phrase operator+(Phrase p, Token t) {
	p += t;
	return p;
}

Phrase operator+(Token t, Phrase p) {
	p.insert(p.begin(), t);
	return p;
}

Phrase operator+(Phrase p1, Phrase p2) {
	for (auto token : p2) p1 += token;
	return p1;
}

// Fail-state query
Issues<Token*> Phrase::issues() const {
	return {/* TODO */};
}

Phrase::operator bool() const {
	return {/* TODO */};
}

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Class constructors
Brief::Brief(StrokeList const& p, Phrase s)
: m_strokeList{p}, m_phrase{s} { normalize(); }

Brief::Brief(Brief const& b, Phrase s)
: m_strokeList{b.m_strokeList}, m_phrase{s} { normalize(); }

// Getters and Setters
StrokeList& Brief::strokeList() {
	return m_strokeList;
}

StrokeList const& Brief::strokeList() const {
	return m_strokeList;
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
	return m_strokeList.issues();
}

Brief::operator bool() const {
	if (m_strokeList.empty() && m_phrase.empty()) return false;
	else return (bool)m_strokeList;
}

// Concatenation
Brief& Brief::operator|=(Brief other) {
	m_strokeList |= other.m_strokeList;
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
	Brief result {b.m_strokeList, str};
	return result += str;
}

// Internal
Brief& Brief::normalize() {
	// Remove empty strokes in m_strokeList.
	m_strokeList.erase(
		std::remove(m_strokeList.begin(), m_strokeList.end(), NoStroke),
		m_strokeList.end()
	);
	// Any extra whitespace is already removed thanks to the Phrase constructor.
	return *this;
}

// StrokeList promotion
Brief operator+(StrokeList p, Phrase str) {
	return Brief {p, ""} + str;
}

Brief operator+(Phrase str, StrokeList p) {
	return str + Brief {p, ""};
}

/* ~~ Plain-Text Entry Iterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <>
void EntryIterator<Plain>::next() {
	if (input->eof()) return finish();
	std::string line {};
	std::getline(*input, line);
	if (std::all_of(line.begin(), line.end(), isWhitespace)) return next();
	auto split = line.find('=');
	if (split == line.npos) return fail("expected '='");
	current = Brief {
		StrokeList {line.substr(0, split)},
		Phrase {line.substr(split+1)},
	};
}

/* ~~ JSON Entry Iterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace {
	struct JsonState {
		enum { FirstChar, Body } value {FirstChar};
	};
}

template <>
void EntryIterator<Json>::setup() {
	parseState = JsonState {};
}

template <>
void EntryIterator<Json>::next() {
	auto& state = std::any_cast<JsonState&>(parseState);

	// TODO: Move this outside the next() function.
	auto parseString = [this] () -> std::string {
		char c {}; std::string result {};
		while (input->get(c) && isWhitespace(c)) /**/;
		if (c != '"') return {};
		while (input->get(c)) {
			if (c == '\\') {
				if (!*input) { fail(); return {}; }
				char c = input->get();
				/**/ if (c == 'b') result += '\b';
				else if (c == 'f') result += '\f';
				else if (c == 'n') result += '\n';
				else if (c == 'r') result += '\r';
				else if (c == 't') result += '\t';
				else if (c == 'u') {
					/* TODO: UTF-8 encoding */
					std::string hex {};
					for (int i=0; i<4; i++) hex += input->get();
					// For the time being let's not discard any data.
					result += hex;
				}
				else result += c;
			}
			else if (c == '"') break;
			else result += c;
		}
		return result;
	};

	if (input->eof()) return finish();
	if (state.value == JsonState::FirstChar) {
		// Make sure the first token is '[' or '{', otherwise the file is not
		// JSON or not interesting.
		while (*input && isWhitespace(input->peek())) input->get();
		if (!*input) return fail("empty JSON file");
		if (input->peek() != '[' && input->peek() != '{') {
			return fail("JSON file does not start with array or object");
		}
		state.value = JsonState::Body;
	}
	std::string stringL {}, stringR {};
	enum { StrL, Colon, StrR, Accept } entryState {StrL};
	while (*input && entryState != Accept) {
		if (entryState == StrL) {
			while (*input && input->peek() != '"') input->get();
			if (!*input) return finish();
			stringL = parseString();
			entryState = Colon;
		}
		else if (entryState == Colon) {
			for (char c; input->get(c); /**/) {
				if (isWhitespace(c)) /**/;
				else if (c == ':') { entryState = StrR; break; }
				else return fail("expected ':'");
			}
		}
		else if (entryState == StrR) {
			stringR = parseString();
			entryState = Accept;
		}
	}
	current = Brief {StrokeList {stringL}, Phrase {stringR}};
}

/* ~~ RTF/CRE Entry Iterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace {
	struct RtfState {
		enum { Header, Body, Final } value {Header};
	};
}

template <>
void EntryIterator<Rtf>::setup() {
	parseState = RtfState {};
}

template <>
void EntryIterator<Rtf>::next() {
	auto& state = std::any_cast<RtfState&>(parseState);

	static constexpr std::string_view magic {R"({\rtf1)"};
	static constexpr std::string_view primer {R"({\*\cxs )"};
	if (state.value == RtfState::Header) {
		static_assert(magic.size() != 0);
		std::string front {};
		for (char c; input->get(c); /**/) {
			front += c;
			if (front.size() == magic.size()) break;
		}
		if (front != magic) return fail();

		unsigned count {0};
		for (char c; input->get(c); /**/) {
			if (count < primer.size()) {
				if (c == primer[count]) count++;
				else count = 0;
			}
			if (count == primer.size()) break;
		}
	}
	state.value = RtfState::Body;

	if (state.value == RtfState::Final) finish();
	else do {
		std::string line {};
		unsigned count {0};
		for (char c; input->get(c); /**/) {
			line += c;
			if (count < primer.size()) {
				if (c == primer[count]) count++;
				else count = 0;
			}
			if (count == primer.size()) break;
		}
		auto ending = line.size() - (over()? 0: primer.size());
		auto split = line.find('}');
		if (split == line.npos) return fail();
		current = Brief {
			StrokeList {line.substr(0, split)},
			Phrase {line.substr(split+1, ending - (split+1))},
		};
		if (over()) state.value = RtfState::Final;
	} while (!over() && current.issues());
}

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace /*detail*/ {
	constexpr struct {
		bool operator()(Brief const& a, Brief const& b) const {
			return a.strokeList() < b.strokeList();
		}
	} EntryCompare {};
}

Dictionary::Dictionary(std::span<Brief const> span) {
	insert(span.begin(), span.end());
}

Dictionary::Dictionary(std::istream& input, FileType type) {
	if (type != NoFileType) {
		// TODO: Figure out how to refactor to remove duplicate code.
		if (type == Plain) {
			EntryIterator<Plain> begin {input}, end {};
			if (begin == end) return;
			*this = Dictionary {begin, end};
		}
		if (type == Json) {
			EntryIterator<Json> begin {input}, end {};
			if (begin == end) return;
			*this = Dictionary {begin, end};
		}
		if (type == Rtf) {
			EntryIterator<Rtf> begin {input}, end {};
			if (begin == end) return;
			*this = Dictionary {begin, end};
		}
	}
	// In order to guess the file type we lose the luxury of being able to
	// iterate our data as it comes in. There's probably an advanced solution
	// which uses the first 100 or so bytes to determine the winning file type
	// and processes the (potentially ginormous) rest of the file that way.
	else {
		std::istreambuf_iterator<char> begin {input}, end {};
		std::string entireFile {begin, end};
		for (auto guess : {Rtf, Json, Plain}) {
			std::istringstream iss {entireFile};
			if (auto result = Dictionary {iss, guess}; !result.issues()) {
				*this = result;
			}
		}
	}
	// TODO: Let the user know the parse failed by returning something other
	// than an empty dictionary.
}

Issues<Brief const*> Dictionary::issues() const {
	return {/* TODO */};
}

void Dictionary::clean() {
	std::erase_if(m_entries, [] (Brief const& b) {
		return b.issues()
		||     b.strokeList() == NoStrokeList
		||     b.phrase() == NoPhrase;
	});
}

// TODO: Optional argument for how to handle insertion,
//       or maybe have the comparator decide what to do.
Dictionary::iterator Dictionary::insert(Brief const& b) {
	// Efficiently find our sorted position
	auto position = std::lower_bound(begin(), end(), b, EntryCompare);
	// Our entry doesn't already exist
	if (position == end() || position->strokeList() != b.strokeList()) {
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

std::size_t Dictionary::erase(StrokeList p) {
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

bool Dictionary::contains(StrokeList const& p) const {
	return find(p) != end();
}

Dictionary::iterator Dictionary::find(StrokeList const& p) {
	auto it = std::lower_bound(begin(), end(), Brief {p, ""}, EntryCompare);
	if (it == end() || it->strokeList() != p) return end();
	else return it;
}

Dictionary::const_iterator Dictionary::find(StrokeList const& p) const {
	auto it = std::lower_bound(begin(), end(), Brief {p, ""}, EntryCompare);
	if (it == end() || it->strokeList() != p) return end();
	else return it;
}

Dictionary::iterator Dictionary::lower_bound(StrokeList const& p) {
	return find(p);
}

Dictionary::const_iterator Dictionary::lower_bound(StrokeList const& p) const {
	return find(p);
}

Dictionary::iterator Dictionary::upper_bound(StrokeList const& p) {
	auto it = find(p);
	if (it != end()) return it+1;
	else return end();
}

Dictionary::const_iterator Dictionary::upper_bound(StrokeList const& p) const {
	auto it = find(p);
	if (it != end()) return it+1;
	else return end();
}

std::pair<Dictionary::iterator, Dictionary::iterator>
Dictionary::equal_range(StrokeList const& p) {
	auto it = find(p);
	if (it != end()) return {it, it+1};
	else return {end(), end()};
}

std::pair<Dictionary::const_iterator, Dictionary::const_iterator>
Dictionary::equal_range(StrokeList const& p) const {
	auto it = find(p);
	if (it != end()) return {it, it+1};
	else return {end(), end()};
}

// Map methods
Phrase& Dictionary::operator[](StrokeList const& p) {
	auto it = find(p);
	if (it != end()) return it->phrase();
	else {
		auto it = emplace(p, NoPhrase);
		return it->phrase();
	}
}

Phrase& Dictionary::at(StrokeList const& p) {
	auto it = find(p);
	if (it != end()) return it->phrase();
	else throw std::out_of_range {toString(p)};
}

Phrase const& Dictionary::at(StrokeList const& p) const {
	auto it = find(p);
	if (it != end()) return it->phrase();
	else throw std::out_of_range {toString(p)};
}

void Dictionary::normalize() {
	std::sort(begin(), end(), EntryCompare);
}

/* ~~ Language Identification ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Getters
std::string LanguageCode::name() const {
	return {m_name.begin(), m_name.end()};
}

std::string LanguageCode::script() const {
	return {m_script.begin(), m_script.end()};
}

std::string LanguageCode::region() const {
	return {m_region.begin(), m_region.end()};
}

//std::array<char, 3+4+2> LanguageCode::raw() const {
//	return std::array {
//		m_name  [0], m_name  [1], m_name  [2],
//		m_script[0], m_script[1], m_script[2], m_script[3],
//		m_region[0], m_region[1],
//	};
//}

/* ~~ Language Definitions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void NoLanguage_Arg::State::applyWord(std::ostream& os, Word word) {
	// Notice the lack of any spacing. NoLanguage is bare to the bone.
	os << std::string_view {word};
}

void NoLanguage_Arg::State::applyPunctuation(
	std::ostream& os,
	std::string_view symbol
) {
	os << symbol;
}

/* ~~ Context Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

LanguageCode Context::languageCode() const {
	return std::visit(
		[] <class S> (S const& state) { return state.Language().Code; },
		m_state
	);
}

Context::AnyState& Context::state() {
	return m_state;
}

Context::AnyState const& Context::state() const {
	return m_state;
}

/* ~~ Speech Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Getters and Setters
Context& Speech::context() {
	return m_context;
}

Context const& Speech::context() const {
	return m_context;
}

Speech& operator<<(Speech& speech, Word const& word) {
	speech << Token {word};
	return speech;
}

Speech& operator<<(Speech& speech, Token const& token) {
	auto& os = *speech.m_output;
	auto& context = speech.m_context;
	auto modify = [&] <class S> (S& state) {
		if (auto const* word = token.word()) {
			state.applyWord(os, *word);
		}
		else if (auto const* signal = token.signal()) {
			if (*signal == NoSignal) /**/;
			// It's the Translator's job to handle the undoing of strokes.
			// However, if this signal still slips though, don't disregard it.
			else if (signal->as(Undo)) speech << Word {"*"};
			else if (signal->as(Cancel)) state = {Default};
			// Complex Signals
			else if (auto const* data = signal->as(Punctuate)) {
				state.applyPunctuation(os, data->symbol);
			}
			else if (auto const* data = signal->as(CodeSwitch)) {
				// Invalidates state reference.
				applyCodeSwitch(context, data->localeName);
			}
			else if (signal->as(SysEx)) {/* Do nothing, with style! */}
		}
	};

	std::visit(modify, context.state());
	return speech;
}

Speech& operator<<(Speech& speech, Phrase const& p) {
	for (auto token : p) speech << token;
	return speech;
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

std::string toString(StrokeList const& p, Format format) {
	std::string result = "";
	for (int i=0; auto stroke : p) {
		if (i++) result += '/';
		result += toString(stroke, format);
	}
	return result;
}

std::string toString(Token const& t) {
	std::string result {};
	if (auto const* word = t.word()) {
		for (char c : *word) result += asEscaped(c);
	}
	if (auto const* signal = t.signal()) {
		/**/ if (*signal == NoSignal) result += "{#}";
		else if (signal->as(Undo))    result += "{*}";
		else if (signal->as(Cancel))  result += "{}";
		else if (auto const* data = signal->as(Punctuate)) {
			result += "{" + data->symbol + "}";
		}
		else if (auto const* data = signal->as(CodeSwitch)) {
			result += "{@" + data->localeName + "}";
		}
		else if (auto const* data = signal->as(SysEx)) {
			result += "{#" + data->channel + ":" + data->message + "}";
		}
	}
	return result;
}

std::string toString(Phrase const& p) {
	std::string result {};
	for (int i=0; auto const& token : p) {
		result += (i++? " ": "") + toString(token);
	}
	return result;
}

std::string toString(Brief const& b, Format format) {
	return toString(b.strokeList(), format) + ", " + toString(b.phrase());
}

std::ostream& operator<<(std::ostream& os, Stroke s) {
	auto format = Format(os.iword(Format_xalloc));
	if (!bits(format)) format = StrokeDefault;
	return os << toString(s, format);
}

std::ostream& operator<<(std::ostream& os, StrokeList const& p) {
	auto format = Format(os.iword(Format_xalloc));
	if (!bits(format)) format = StrokeDefault;
	return os << toString(p, format);
}

std::ostream& operator<<(std::ostream& os, Token const& t) {
	return os << toString(t);
}

std::ostream& operator<<(std::ostream& os, Phrase const& p) {
	return os << toString(p);
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

using namespace steno;

std::size_t std::hash<Stroke>::operator()(Stroke const& x) const {
	return std::hash<uint32_t> {} (x.m_bits);
}

// https://stackoverflow.com/a/72073933
std::size_t std::hash<StrokeList>::operator()(StrokeList const& x) const {
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
