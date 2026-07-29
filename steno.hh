/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
|* ~  C++ Steno Library                                                     ~ *|
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ https://en.wikipedia.org/wiki/Stenotype ~~ */

//   The C++ Steno Library provides classes for three functionalities: keyboard,
// text, and language. This header file includes all classes (types) shown in
// the following diagram, as well as utility classes (Issues, Format, etc.)

//   Keyboard classes concern the English stenotype layout. This is a special
// 23-key keyboard which is used the same way one would play chords on a piano.

//   Text classes contain the data for sentence fragments and punctuation. They
// require a context to be manipulated, i.e. which language's rules to follow.

//   Language classes contain region-specific rules for orthography. This allows
// international developers to add in support for their own languages.



// The following diagram can be read like so:
// -	"Dictionary is a struct that has a list of Briefs"
// -	"Speech is a struct that has a list of Tokens and Context"
// -	"Token is a union that has either a Word or a Signal"

/* ┌─────────────────┬─────────────────────────────────────────┬────────────┐ *\
|*                                                                            *|
|*              ┌──────────┐                                   .              *|
|*              │Dictionary│                                                  *|
|*              └──────────┘                                   .              *|
|*                   ▲                                                        *|
|*                   │                                         .              *|
|*                   ≡                                                        *|
|*                   │                                         .              *|
|*                ┌─────┐                       ┌──────┐                      *|
|*                │Brief│                       │Speech│       .              *|
|*                └─────┘                       └──────┘                      *|
|*                   ▲                             ▲           .              *|
|*                   │                             │                          *|
|*       ┌───────────┴───────────┐     ┌───────────┴───────────┐              *|
|*       │                       │     │                       │              *|
|*  ┌──────────┐     .        ┌──────┐ │                   ┌───────┐          *|
|*  │StrokeList│              │Phrase│ │                   │Context│          *|
|*  └──────────┘     .        └──────┘ │                   └───────┘          *|
|*       ▲                         ▲   │                       ▲              *|
|*       │           .             │   │                       │              *|
|*       ≡                         ≡   ≡               ┌───────┴───────┐      *|
|*       │           .             │   │               │       .       │      *|
|*    ┌──────┐                    ┌─────┐           ┌─────┐         ┌──────┐  *|
|*    │Stroke│       .            │Token│           │State│    .    │Locale│  *|
|*    └──────┘                    └─────┘           └─────┘         └──────┘  *|
|*       ▲           .               ▲                         .              *|
|*       │                           ║                                        *|
|*       ≡           .       ╓───────╨───────╖                 .              *|
|*       │                   ║               ║                                *|
|*     ┌───┐         .     ┌────┐         ┌──────┐             .              *|
|*     │Key│               │Word│         │Signal│                            *|
|*     └───┘         .     └────┘         └──────┘             .              *|
|*                                                                            *|
|* └────Keyboard─────┴───────────────────Text──────────────────┴──Language──┘ *|
|*                                                                            *|
|*  ┌───┐          ▲            ▲           │                                 *|
|*  │   │ type     │ struct     ║ union     ≡ container                       *|
\*  └───┘          │            ║           │                                 */

#pragma once
#include <algorithm>
#include <any>
#include <array>
#include <bit>
#include <bitset>
#include <concepts>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>
#include <cstdint>
#include <cassert>

namespace steno {

/* ~~ Utilities ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace flags {
	// Stroke construction
	constexpr struct FromBits_Arg         {} FromBits         {};
	constexpr struct FromBitsReversed_Arg {} FromBitsReversed {};
	// Signal construction
	constexpr struct Cancel_Arg           {} Cancel           {};
	constexpr struct Undo_Arg             {} Undo             {};
	constexpr struct Punctuate_Arg        {} Punctuate        {};
	constexpr struct CodeSwitch_Arg       {} CodeSwitch       {};
	constexpr struct SysEx_Arg            {} SysEx            {};
	// Context/State construction
	constexpr struct Default_Arg          {} Default          {};
	constexpr struct Opening_Arg          {} Opening          {};
}

using namespace flags;

template <class T>
struct Issues : std::vector<T> {
	using std::vector<T>::vector;
	operator bool() const { return !this->empty(); }
};
// TODO: NoIssues object which acts like std::nullopt

template <class ... Ts>
struct TypeList {
	static constexpr std::size_t Size {sizeof ... (Ts)};

	template <std::size_t I>
	using At = std::tuple_element_t<I, std::tuple<Ts ... >>;

	template <template <class> class MetaFunction>
	using Map = TypeList<MetaFunction<Ts> ... >;

	template <template <class ... > class Container>
	using In = Container<Ts ... >;
};

template <auto ... Vs>
struct ValueList {
	using Types = TypeList<decltype(Vs) ... >;
	static constexpr std::size_t Size {sizeof ... (Vs)};

	template <std::size_t I>
	static constexpr auto At {std::get<I>(std::tuple {Vs ... })};

	static void ForEach(auto&& function) {
		[&] <std::size_t ... I> (std::index_sequence<I ... >) {
			(function(At<I>), ... );
		} (std::make_index_sequence<Size> {});
	}
};

/* ~~ Key ID's ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Keys are bit-fields that identify their part of the Stroke::m_bits integer.
// Their order is based on the physical layout of a steno keyboard.

enum class Key : uint32_t {
	// Number Bar
	Num = 1u<<31,
	// Initial Consonants
	S_ = 1u<<30,
	T_ = 1u<<29, K_ = 1u<<28,
	P_ = 1u<<27, W_ = 1u<<26,
	H_ = 1u<<25, R_ = 1u<<24,
	// Vowels & Asterisk
	A  = 1u<<23, O  = 1u<<22,
	x  = 1u<<21,
	E  = 1u<<20, U  = 1u<<19,
	// Final Consonants
	_F = 1u<<18, _R = 1u<<17,
	_P = 1u<<16, _B = 1u<<15,
	_L = 1u<<14, _G = 1u<<13,
	_T = 1u<<12, _S = 1u<<11,
	_D = 1u<<10, _Z = 1u<< 9,
	// Flags
//	Mark = 1<<8,
//	Fail = 1<<0,
};

/* ~~ Stroke Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Strokes represent a subset of the steno keyboard pressed simultaneously.
// They can be manipulated like bitsets, or treated as a container of Keys.

class Stroke {
	uint32_t m_bits
	//  #STKPWHRAO*EUFRPBLGTSDZ !        X
	= 0b00000000000000000000000'00000000'0;
	//  └──────────┬──────────┘ └──┬───┘ └─ fail-bit
	//            keys        flags/resv'd

public:
	static constexpr unsigned KeyCount = 23;
	static constexpr unsigned PadCount = 9;
	static_assert(KeyCount + PadCount == 32);

	// Default construction/assignment
	Stroke() = default;
	Stroke(Stroke const&) = default;
	Stroke& operator=(Stroke const&) = default;

	// Class constructors
	constexpr Stroke(Key);
	constexpr Stroke(std::string_view);
	constexpr Stroke(FromBits_Arg, std::bitset<23> const);
	constexpr Stroke(FromBitsReversed_Arg, std::bitset<23> const);
	template <std::input_iterator I> constexpr Stroke(I, I);

	// Getters and Setters
	class Reference;
	class Iterator;
	uint32_t bits() const;
	bool get(Key) const;
	Stroke& set(Key, bool = true);
	Stroke& unset(Key);
	Reference operator[](Key);
	bool operator[](Key) const;
	Stroke& clear();

	// Range-for compatibility
	Iterator begin() const;
	Iterator end() const;

	// Comparison
	bool operator==(Stroke const&) const = default;
	auto operator<=>(Stroke const&) const = default;
	template <class T> friend struct std::hash;

	// Key manipulation
	Stroke operator~() const;
	Stroke& operator+=(Stroke const&);
	Stroke& operator-=(Stroke const&);
	Stroke& operator&=(Stroke const&);
	Stroke& operator^=(Stroke const&);
	friend Stroke operator+(Stroke, Stroke const&);
	friend Stroke operator-(Stroke, Stroke const&);
	friend Stroke operator&(Stroke, Stroke const&);
	friend Stroke operator^(Stroke, Stroke const&);

	// Fail-state query
	Issues<Key> issues() const;
	operator bool() const;

	// Key proxy class
	class Reference {
		friend class Stroke;
		Stroke* parent;
		Key key;

	public:
		Reference(Reference const&) = default;
		Reference(Stroke* p, Key k): parent{p}, key{k} {}
		operator bool() const;
		Reference& operator=(bool);
		Reference& operator=(Reference const&);
	};

	class Iterator {
		friend class Stroke;
		// Matches the bits of this Stroke.
		// The leading bit => the Key the iterator "points" to.
		uint32_t m_bits = 0;

	public:
		Iterator() = default;
		Iterator(Stroke const& s): m_bits{s.m_bits} {}
		bool operator==(Iterator const&) const;
		Iterator& operator++();
		Iterator operator++(int);
		Key operator*() const;
		using difference_type = int;
		using value_type = Key;
	};
	// TODO: Figure out why this assert doesn't work.
//	static_assert(std::indirectly_readable<Iterator>);

private:
	static constexpr auto
	//            #STKPWHRAO*EUFRPBLGTSDZ !        X
	FailBit   = 0b00000000000000000000000'00000000'1,
	FlagsMask = 0b00000000000000000000000'11111111'1;

	uint32_t getFlags() const;
	void setFlags(uint32_t);
};
static constexpr auto NoStroke = Stroke {};

// Key promotion
Stroke operator~(Key);
Stroke operator+(Key, Key);
Stroke operator-(Key, Key);
Stroke operator&(Key, Key);
Stroke operator^(Key, Key);

/* ~~ StrokeList Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// StrokeLists are sequences of strokes.

class StrokeList : public std::vector<Stroke> {
public:
	// Default construction/assignment
	using std::vector<Stroke>::vector;

	// Class constructors
	StrokeList(std::string_view);
	StrokeList(Stroke);
	StrokeList(std::span<Stroke const>);

	// Comparison
	bool operator== (StrokeList const&) const = default;
	auto operator<=>(StrokeList const&) const = default;

	// Concatenation
	StrokeList& operator|=(StrokeList);
	friend StrokeList operator|(StrokeList, StrokeList const&);

	// Fail-state query
	Issues<Stroke const*> issues() const;
	operator bool() const;

	// Vector methods
	friend void erase   (StrokeList& p, auto&& value);
	friend void erase_if(StrokeList& p, auto&& pred);

private:
	void erase_impl(auto&& value)
	{ erase(std::remove(begin(), end(), value), end()); }
	void erase_if_impl(auto&& pred)
	{ erase(std::remove_if(begin(), end(), pred), end()); }
};
static auto const NoStrokeList = StrokeList {};

// Stroke promotion
StrokeList operator|(Stroke, Stroke const&);

/* ~~ Word Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Words (more correctly morphemes) are literal character data.
// Their constructor ignores escape sequences, parsing those is the job of the
// Phrase constructor.

using Word = std::string;
static auto const NoWord = Word {};

/* ~~ Signal Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Signals are instructions to modify the current context.
// They can describe, for example, how to capitalize the next Word, how to join
// with adjacent Words, text formatting, or even system exclusive messages.

class Signal {
#define SIGNAL_DEF(Name, ... )                                                 \
    struct Name##_t {                                                          \
        __VA_ARGS__                                                            \
        bool operator== (Name##_t const&) const = default;                     \
        auto operator<=>(Name##_t const&) const = default;                     \
    }
#define SIGNAL_MEMBERS(Name)                                                   \
    constexpr                                                                  \
    Signal(Name##_Arg, auto&& ... args): m_value{Name##_t {args ... }} {}      \
    auto as(Name##_Arg)       { return std::get_if<Name##_t>(&m_value); }      \
    auto as(Name##_Arg) const { return std::get_if<Name##_t>(&m_value); }

	/*         Name      Comment                                Example       */
	/* (default init) */ // Will be ignored by all systems.     {#}
	SIGNAL_DEF(Undo   ); // Forget the previous Stroke.         {*}
	SIGNAL_DEF(Cancel ); // We are at the start of a word.      {}

	// Language-specific formatting                             {!}
	SIGNAL_DEF(Punctuate, std::string symbol {}; );

	// Accepts POSIX locales and Language IDs (Apple)           {@en-US}
	SIGNAL_DEF(CodeSwitch, std::string localeName {}; );

	// Application-specific instructions. Your own universe!    {#MyApp:Reload}
	SIGNAL_DEF(SysEx, std::string channel {}, message {}; );
#undef SIGNAL_DEF

	std::variant<
		std::monostate, Undo_t, Cancel_t,
		Punctuate_t, CodeSwitch_t, SysEx_t
	> m_value {};

public:
	// Default construction/assignment
	Signal() = default;
	Signal(Signal const&) = default;
	Signal& operator=(Signal const&) = default;

	// Tagged constructors      API example
	SIGNAL_MEMBERS(Undo);       // Signal {Undo}
	SIGNAL_MEMBERS(Cancel);     // Signal {Cancel}
	SIGNAL_MEMBERS(Punctuate);  // Signal {Punctuate, "!"}
	SIGNAL_MEMBERS(CodeSwitch); // Signal {CodeSwitch, "en-US"}
	SIGNAL_MEMBERS(SysEx);      // Signal {SysEx, "MyApp", "Reload"}

	// Comparison
	bool operator== (Signal const&) const = default;
	auto operator<=>(Signal const&) const = default;

	operator bool() const;
#undef SIGNAL_DEF
#undef SIGNAL_MEMBERS
};
static auto const NoSignal = Signal {};

/* ~~ Token Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Tokens are the smallest unit of speech for both humans and computers.
// Individual prefixes and suffixes count as Words, and Signals appear in series
// rather than multiple at the same time.

class Token : public std::variant<Signal, Word> {
public:
	// Constructors
	using std::variant<Signal, Word>::variant;

	// Comparison
	bool operator== (Token const&) const = default;
	auto operator<=>(Token const&) const = default;

	// Getters
	Word /* */* word();
	Word const* word() const;
	Signal /* */* signal();
	Signal const* signal() const;
};
static auto const NoToken = Token {};

/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Phrases are constructed from strings with signals embedded inside them.
// It is the Phrase's job to parse this information, but not apply any
// orthography rules. On their own they act independently of context.

class Phrase : public std::vector<Token> {
public:
	// Parse steno string
	Phrase(std::string_view="");
	template <std::size_t N> Phrase(char const (& str)[N])
	:	Phrase{std::string_view {str}} {}

	// Construct from Token sequence
	template <std::input_iterator I> Phrase(I first, I last)
	:	std::vector<Token>{first, last} {}

	// Comparison
	bool operator== (Phrase const&) const = default;
	auto operator<=>(Phrase const&) const = default;

	// Concatenation
	Phrase& operator+=(Phrase);
	Phrase& operator+=(Token);
	friend Phrase operator+(Phrase, Token);
	friend Phrase operator+(Token, Phrase);
	// Disambiguate string literals, prefer Phrases
	template <std::size_t N>
	Phrase& operator+=(char const (& str)[N]) { return *this += Phrase {str}; }

	// Fail-state query
	Issues<Token*> issues() const;
	operator bool() const;
};
static auto const NoPhrase = Phrase {};

/* ~~ Brief Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Briefs associate StrokeLists and Phrases, and can be modified like either.
// They are primarily used as Dictionary entries.

class Brief {
	StrokeList m_strokeList {};
	Phrase m_phrase {};

public:
	// Default construction/assignment
	Brief() = default;
	Brief(Brief const&) = default;
	Brief& operator=(Brief const&) = default;

	// Class constructors
	Brief(StrokeList const&, Phrase);
	Brief(Brief const&, Phrase);

	// Getters and Setters
	StrokeList /* */& strokes();
	StrokeList const& strokes() const;
	Phrase /* */& phrase();
	Phrase const& phrase() const;
	template <std::size_t I> friend auto&& get(Brief&);
	template <std::size_t I> friend auto&& get(Brief const&);
	template <std::size_t I> friend auto&& get(Brief&&);
	Brief& clear();

	// Comparison
	bool operator== (Brief const&) const = default;
	auto operator<=>(Brief const&) const = default;

	// Concatenation
	Brief& operator|=(Brief);
	friend Brief operator|(Brief, Brief const&);
	Brief& operator+=(Phrase);
	friend Brief operator+(Brief, Phrase);
	friend Brief operator+(Phrase, Brief);

	// Fail-state query
	// TODO: Figure out how to report phrase issues.
	Issues<Stroke const*> issues() const;
	operator bool() const;

private:
	Brief& normalize();

#define GET_IMPL(Attr) \
	template <std::size_t I> auto&& get_impl() Attr {                          \
	    if constexpr (I==0) return m_strokeList;                               \
	    if constexpr (I==1) return m_phrase;                                   \
	}
	GET_IMPL(&) GET_IMPL(const&) GET_IMPL(&&)
#undef GET_IMPL
};
static auto const NoBrief = Brief {};

/* ~~ Entry Iterator Classes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   EntryIterators parse steno dictionary files. This means we don't have
// to wait for the end of the file to process our data. For most formats this is
// easy to implement. But worst case scenario, the 'setup' function can run a
// first pass, store everything in 'state', then iterator on a second pass.

enum FileType {
	NoFileType,
	Plain, Json, Rtf,
};

template <FileType FT>
class EntryIterator {
	std::istream* input {nullptr};
	Brief current {};

	// TODO: Flesh out SourceLocation class.
	using SourceLocation = std::shared_ptr<std::string>;
	Issues<SourceLocation> issueLocations {};

	std::any parseState {};
	void setup() {}
	void next();

public:
	using value_type = Brief;
	using difference_type = std::ptrdiff_t;

	EntryIterator() = default;
	EntryIterator(EntryIterator const&) = default;
	EntryIterator& operator=(EntryIterator const&) = default;

	EntryIterator(std::istream& in): input{&in} { setup(); next(); }

	Issues<SourceLocation> const& issues() const { return issueLocations; }

	bool operator==(EntryIterator const& other) const {
		return this->over() && other.over();
	}

	Brief operator*() const { return current; }
	EntryIterator& operator++() { next(); return *this; }

	EntryIterator operator++(int) {
		EntryIterator old = *this;
		++(*this);
		return old;
	}

private:
	void finish() {
		input = nullptr;
	}

	bool over() const {
		return input == nullptr;
	}

	void fail(std::string message={}) {
		issueLocations.push_back(std::make_shared<std::string>(message));
		input = nullptr;
	}

	static constexpr bool isWhitespace(char c) {
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}
};

/* ~~ Dictionary Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Dictionaries provide fast look-up and insertion for many Briefs.
// They behave like std::map<StrokeList, Phrase>, with Brief value_types.

class Dictionary {
	std::deque<Brief> m_entries {};

public:
	// Default construction/assignment/movement
	Dictionary() = default;
	Dictionary(Dictionary const&) = default;
	Dictionary(Dictionary&&/* */) = default;
	Dictionary& operator=(Dictionary const&) = default;
	Dictionary& operator=(Dictionary&&/* */) = default;

	// Class constructors
	Dictionary(std::span<Brief const>);
	Dictionary(std::istream&, FileType=NoFileType);

	// Comparison
	bool operator== (Dictionary const&) const = default;
	auto operator<=>(Dictionary const&) const = default;

	// Fail-state query
	Issues<Brief const*> issues() const;
	void clean();

public:
	// Container types
	using value_type = Brief;
	using reference = Brief&;
	using const_reference = Brief const&;
	using pointer = Brief*;
	using const_pointer = Brief const*;
	using iterator = decltype(m_entries)::iterator;
	using const_iterator = decltype(m_entries)::const_iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;

	// Container methods
#define USE(Name) \
	auto    Name()       { return m_entries.   Name(); } \
	auto    Name() const { return m_entries.   Name(); } \
	auto c##Name() const { return m_entries.c##Name(); }
	USE(begin) USE(end) USE(rbegin) USE(rend)
#undef USE
	void swap(Dictionary& other) { std::swap(*this, other); };
	std::size_t size    () const { return m_entries.size    (); }
	std::size_t max_size() const { return m_entries.max_size(); }
	bool        empty   () const { return m_entries.empty   (); }

public:
	// Associative types
	using key_type = StrokeList;
	using mapped_type = Phrase;

	// Associative methods
	iterator insert(Brief const&);
	template <std::input_iterator I>
	Dictionary(I first, I last): m_entries(first, last) { normalize(); }
	Dictionary(std::initializer_list<Brief> il): m_entries(il) { normalize(); };
	template <std::input_iterator I>
	void insert(I i, I j) { for (I it=i; it!=j; ++it) insert(*it); }
	void insert(std::initializer_list<Brief>);
	iterator emplace(auto&& ... args) { return insert(Brief {args ... }); }
	std::size_t erase(StrokeList);
	iterator erase(const_iterator);
	iterator erase(const_iterator, const_iterator);
	void merge(Dictionary&);
	void merge(Dictionary&&);
	void clear();
	bool contains(StrokeList const&) const;
	/*  */iterator find(StrokeList const&);
	const_iterator find(StrokeList const&) const;
	/*  */iterator lower_bound(StrokeList const&);
	const_iterator lower_bound(StrokeList const&) const;
	/*  */iterator upper_bound(StrokeList const&);
	const_iterator upper_bound(StrokeList const&) const;
	std::pair<iterator, iterator>
	equal_range(StrokeList const&);
	std::pair<const_iterator, const_iterator>
	equal_range(StrokeList const&) const;

	// Map methods
	Phrase& operator[](StrokeList const&);
	Phrase const& operator[](StrokeList const&) const;
	Phrase& at(StrokeList const&);
	Phrase const& at(StrokeList const&) const;
	friend void erase   (Dictionary& p, auto&& value);
	friend void erase_if(Dictionary& p, auto&& pred);

private:
	void normalize();
	void erase_if_impl(auto&& pred)
	{ for (auto it=begin(); it!=end(); ++it) if (pred(*it)) erase(it); }
	void erase_impl(auto&& value)
	{ erase_if_impl([&] (auto y) { return value == y; }); }
};
static auto const NoDictionary = Dictionary {};

/* ~~ Language Identification ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Scripts are a mandatory part of language identification. This is okay
// because this library only handles the written word, and never any semantic
// meaning. Region, however, is only to be used when two cultures' use of a
// language differ so much that their combining rules differ. Most international
// languages have identical rules, so this is rarely needed.

class LanguageCode {
	// By default we use reserved values to denote lack of code/script/region.
	std::array<char, 3> m_name   {'q','a','a'    }; // ISO 639-2/T
	std::array<char, 4> m_script {'Q','a','a','a'}; // ISO 15924
	std::array<char, 2> m_region {'A','A'        }; // ISO 3166-1 alpha-2

public:
	// Constructors
	consteval LanguageCode() = default;
	consteval LanguageCode(
		std::string_view name,
		std::string_view script,
		std::string_view region={}
	) {
		for (int i=0; i<3; i++) m_name  [i] = name  [i];
		for (int i=0; i<4; i++) m_script[i] = script[i];
		if (region.empty()) return;
		for (int i=0; i<2; i++) m_region[i] = region[i];
	}

	// Comparison
	bool operator== (LanguageCode const&) const = default;
	auto operator<=>(LanguageCode const&) const = default;

	// Getters
	std::string name  () const;
	std::string script() const;
	std::string region() const;
};
static constexpr auto NoLanguageCode = LanguageCode {};

/* ~~ Language Definitions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Language objects act similar to both namespaces and template arguments.
// The "_Arg" struct contains all the details. However, only the object
// with the language's name itself is meant to be used in the API.
//   Examples:
// speak(English); // Template-like usage (struct tag)
// English.Capitalize; // Namespace-like usage

template <class T>
concept Language_Arg = std::is_empty_v<T> && requires {
	{ T::Name } -> std::convertible_to<std::string_view>;
	{ T::Code } -> std::convertible_to<LanguageCode>;
	requires std::regular<typename T::State>;
	typename T::State {Default};
	typename T::State {Opening};
};

// Bare-bones Language implementation
struct NoLanguage_Arg {
	static constexpr std::string_view Name {"(no language)"};
	static constexpr LanguageCode Code {NoLanguageCode};
	struct State {
		static constexpr auto Language() { return NoLanguage_Arg {}; };

		State(Default_Arg={}) {};
		State(Opening_Arg) {};
		auto operator<=>(State const&) const = default;

		// TODO: Put these in Language_Arg concept.
		void applyWord(std::ostream&, Word);
		void applyPunctuation(std::ostream&, std::string_view);
	};
};
static constexpr auto NoLanguage = NoLanguage_Arg {};

// Useful functions
template <class T>
constexpr bool IsLanguage(T) { return Language_Arg<T>; }

// TODO: Decide if Languages should be truthy based on if they equal NoLanguage.
template <Language_Arg L>
constexpr bool operator==(L, L) { return true; }

template <Language_Arg L1, Language_Arg L2>
constexpr bool operator==(L1, L2) { return false; }

} // namespace steno
#include "steno_languages.hh"
namespace steno {

/* ~~ Context Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Contexts store everything implied about what is being written.
// For example, before a stenographer starts writing, we know we are in English,
// and the first word will be treated as the start of the sentence/paragraph.

class Context {
	using AnyState = Languages::States::In<std::variant>;
	AnyState m_state {};

public:
	// Constructors
	Context(): Context{DefaultLanguage} {}
	Context(Default_Arg): Context{DefaultLanguage, Default} {}
	Context(Opening_Arg): Context{DefaultLanguage, Opening} {}

	Context(Language_Arg auto Language): Context{Language, Default} {}
	template <Language_Arg L> Context(L, auto StartingState)
	:	m_state{typename L::State {StartingState}} {}

	// Getters and Setters
	LanguageCode languageCode() const;
	template <Language_Arg L> Context& codeSwitch(L);

	AnyState /* */& state();
	AnyState const& state() const;
	template <Language_Arg L> L::State /* */* as(L);
	template <Language_Arg L> L::State const* as(L) const;

	// Comparison
	bool operator== (Context const&) const = default;
	auto operator<=>(Context const&) const = default;
};
static auto const NoContext = Context {};

/* ~~ Speech Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Speeches listen for Tokens, apply orthography, and output to std::ostream.
// Information received will always be sent out as fast as possible. There is
// however, no ability to undo nor reinterpret Tokens.

class Speech {
	std::ostream* m_output {};
	Context m_context {};

public:
	// Constructors
	Speech(std::ostream& os): Speech{os, DefaultLanguage} {}
	Speech(std::ostream& os, auto Language): Speech{os, Language, Opening} {}
	Speech(std::ostream& os, auto Language, auto StartingState)
	:	m_output{&os}, m_context{Language, StartingState} {}

	// Getters and Setters
	Context /* */& context();
	Context const& context() const;

	friend Speech& operator<<(Speech&, Token const&);
	friend Speech& operator<<(Speech&, Phrase const&);
};

// Disambiguate string literals, prefer Phrases
template <std::size_t N>
Speech& operator<<(Speech& s, char const (& str)[N]) {
	return s << Phrase {str};
}

/* ~~ String Output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

//   Formats can be used with std::ostream or passed as function arguments.
// Right now they only specify the output of Strokes.

// We use 'long' for ios_base::iword compatibility.
enum class Format : long {
	// Width                XX
	Packed        = 0b00'00'01,
	Wide          = 0b00'00'10,
	// Hyphen            XX
	Hyphen        = 0b00'01'00,
	NoHyphen      = 0b00'10'00,
	// Digit          XX
	Numeric       = 0b01'00'00,
	Alpha         = 0b10'00'00,

	StrokeDefault = 0b01'01'01,
	KeyDefault    = 0b00'10'00,
	// TODO: More formatting options for StrokeLists and upward
};
using enum Format;
Format operator|(Format, Format);
Format operator|=(Format&, Format);

char toChar(Key);
char toCharShift(Key);
std::string toString(Key, Format = KeyDefault);
std::string toString(Stroke, Format = StrokeDefault);
std::string toString(StrokeList const&, Format = StrokeDefault);
std::string toString(Token const&);
std::string toString(Phrase const&);
std::string toString(Brief const&, Format = StrokeDefault);
std::ostream& operator<<(std::ostream&, Key);
std::ostream& operator<<(std::ostream&, Stroke);
std::ostream& operator<<(std::ostream&, StrokeList const&);
std::ostream& operator<<(std::ostream&, Token const&);
std::ostream& operator<<(std::ostream&, Phrase const&);
std::ostream& operator<<(std::ostream&, Brief const&);

// Format as manipulator
std::ostream& operator<<(std::ostream&, Format);

/* ~~ Constexpr/Template Definitions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr Stroke::Stroke(std::string_view str) {
	enum State {
//		Mk, /*!*/
		Nm, /*#*/
		S_, T_, K_, P_, W_, H_, R_,
		A , O , x , E , U ,
		_F, _R, _P, _B, _L, _G, _T, _S, _D, _Z,
		Begin = Nm, End = _Z+1
	};

	auto next = [] (State s) { return (s == End)? End: State(int(s)+1); };
	auto in = [] (char c, std::string_view s) { return s.find(c) != s.npos; };
	bool const Numeric = std::all_of(
		str.begin(), str.end(),
		[&] (char c) { return in(c, " \t" "#0123456789"); }
	);

	//   On the left is every state's possible next valid input. This creates
	// a broken triangle we can play billiards on to parse our string.
	//   On the right we play this game with the example input "SPROUTS".
	// Advancing in the X direction <=> finding next valid input.
	// Advancing in the Y direction <=> advancing in the switch statement.

	// S_:    STKPWHRAO*EU-                     █TKPWHRAO*EU-
	// T_:     TKPWHRAO*EU-                     █──█WHRAO*EU-
	// K_:      KPWHRAO*EU-                       K│WHRAO*EU-
	// P_:       PWHRAO*EU-                        │WHRAO*EU-
	// W_:        WHRAO*EU-                        █──█AO*EU-
	// H_:         HRAO*EU-                          H│AO*EU-
	// R_:          RAO*EU-                           │AO*EU-
	// A :           AO*EU-                           █─█*EU-
	// O :            O*EUFRPBLGTSDZ                    │*EUFRPBLGTSDZ
	// x :             *EUFRPBLGTSDZ                    █──█FRPBLGTSDZ
	// E :              EUFRPBLGTSDZ                      E│FRPBLGTSDZ
	// U :               UFRPBLGTSDZ                       │FRPBLGTSDZ
	// _F:                FRPBLGTSDZ                       █──────█SDZ
	// _R:                 RPBLGTSDZ                         RPBLG│SDZ
	// _P:                  PBLGTSDZ                          PBLG│SDZ
	// _B:                   BLGTSDZ                           BLG│SDZ
	// _L:                    LGTSDZ                            LG│SDZ
	// _G:                     GTSDZ                             G│SDZ
	// _T:                      TSDZ                              │SDZ
	// _S:                       SDZ                              ██DZ
	// _D:                        DZ                               █──End;
	// _Z:                         Z                                 Z

	bool valid = false;
	for (State state {Begin}; char c : str) if (!in(c, " \t")) {
		valid = true;
		auto require = [&] (bool condition) { return !(valid &= condition); };
		auto accept = [&] (State s, Key k, char cKey, char cNum = '\0') {
			bool match = (c == cKey) || (c == cNum);
			if (c == cKey) this->m_bits |= (uint32_t)k;
			if (c == cNum) this->m_bits |= (uint32_t)k | (uint32_t)Key::Num;
			if (match) state = next(s);
			return match;
		};
		switch (state) {
			using enum State;
//			case Mk: if (accept(End,Key::Mark    ,'!')); else
			case Nm: if (accept(Nm, Key::Num,'#', '#')); else
			case S_: if (accept(S_, Key::S_, 'S', '1')); else
			case T_: if (accept(T_, Key::T_, 'T', '2')); else
			case K_: if (accept(K_, Key::K_, 'K'     )); else
			case P_: if (accept(P_, Key::P_, 'P', '3')); else
			case W_: if (accept(W_, Key::W_, 'W'     )); else
			case H_: if (accept(H_, Key::H_, 'H', '4')); else
			case R_: if (accept(R_, Key::R_, 'R'     )); else
			case A : if (accept(A , Key::A , 'A', '5')); else
			if (c == '-') state = _F;                    else
			if (require(Numeric || in(c, "O*EU" "0")));  else
			case O : if (accept(O , Key::O , 'O', '0')); else
			case x : if (accept(x , Key::x , '*'     )); else
			case E : if (accept(E , Key::E , 'E'     )); else
			case U : if (accept(U , Key::U , 'U'     )); else
			case _F: if (accept(_F, Key::_F, 'F', '6')); else
			case _R: if (accept(_R, Key::_R, 'R'     )); else
			case _P: if (accept(_P, Key::_P, 'P', '7')); else
			case _B: if (accept(_B, Key::_B, 'B'     )); else
			case _L: if (accept(_L, Key::_L, 'L', '8')); else
			case _G: if (accept(_G, Key::_G, 'G'     )); else
			case _T: if (accept(_T, Key::_T, 'T', '9')); else
			case _S: if (accept(_S, Key::_S, 'S'     )); else
			case _D: if (accept(_D, Key::_D, 'D'     )); else
			case _Z: if (accept(_Z, Key::_Z, 'Z'     )); else
			default: valid = false;
		}
		if (!valid) break;
	}
	if (!valid) m_bits |= FailBit;
}

constexpr Stroke::Stroke(Key k) {
	this->m_bits = (uint32_t)k;
}

constexpr Stroke::Stroke(FromBits_Arg, std::bitset<23> const b) {
	for (unsigned i=0; i<b.size(); i++) if (b[i]) {
		this->m_bits |= 1 << (i + Stroke::PadCount);
	}
}

constexpr Stroke::Stroke(FromBitsReversed_Arg, std::bitset<23> const b) {
	for (unsigned i=0; i<b.size(); i++) if (b[22-i]) {
		this->m_bits |= 1 << (i + Stroke::PadCount);
	}
}

template <std::input_iterator I>
constexpr Stroke::Stroke(I first, I last) {
	for (auto key=first; key!=last; ++key) {
		this->m_bits |= (uint32_t)*key;
	}
}

template <Language_Arg L> Context& Context::codeSwitch(L) {
	// TODO: Better code switching. Carry over as much state as we can.
	m_state = typename L::State {};
	return *this;
}

template <Language_Arg L> L::State /* */* Context::as(L) {
	return std::get_if<typename L::State>(&m_state);
}

template <Language_Arg L> L::State const* Context::as(L) const {
	return std::get_if<typename L::State>(&m_state);
}

} // namespace steno

/* ~~ Misc. STL Functionality ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <> struct std::hash<steno::Stroke>
{ std::size_t operator()(steno::Stroke const&) const; };

template <> struct std::hash<steno::StrokeList>
{ std::size_t operator()(steno::StrokeList const&) const; };

template <> struct std::tuple_size<steno::Brief>
: std::integral_constant<size_t, 2> {};

template <std::size_t I> struct std::tuple_element<I, steno::Brief>
: std::conditional<I == 0, steno::StrokeList, steno::Phrase>
{ static_assert(I < 2); };

namespace steno {
void erase   (StrokeList& t, auto&& x) { t.erase_impl(x);    }
void erase_if(StrokeList& t, auto&& f) { t.erase_if_impl(f); }
void erase   (Dictionary& t, auto&& x) { t.erase_impl(x);    }
void erase_if(Dictionary& t, auto&& f) { t.erase_if_impl(f); }
template <std::size_t I> auto&& get(Brief&       b) { return b.get_impl<I>(); }
template <std::size_t I> auto&& get(Brief const& b) { return b.get_impl<I>(); }
template <std::size_t I> auto&& get(Brief&&      b) { return b.get_impl<I>(); }
}
