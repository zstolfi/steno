#pragma once
#include "steno.hh"
#include <istream>
#include <optional>
#include <memory>

namespace steno {

enum FileType {
	NoFileType,
	Plain, Json, Rtf,
};

// Parsing steno dictionaries is done with iterators. This means we don't have
// to wait for the end of the file to process our data. For most formats this is
// easy to implement. But worst case scenario, the 'setup' function can run a
// first pass, store everything in 'state', and we iterate through that.

template <FileType FT>
class EntryIterator {
	std::istream* input {};
	Brief current {};

	// TODO: Flesh out SourceLocation class.
	using SourceLocation = std::shared_ptr<std::string>;
	Issues<SourceLocation> issueLocations {};

	struct PlainState {};
	struct JsonState { enum { FirstChar, Body } value {FirstChar}; };
	struct RtfState { enum { Header, Body, Final } value {Header}; };
	using State =
		std::conditional_t<FT == Plain, PlainState,
		std::conditional_t<FT == Json, JsonState,
		std::conditional_t<FT == Rtf, RtfState,
	void>>>;

	State state {};
	void setup() {}
	void next();

public:
	using value_type = Brief;
	using difference_type = std::ptrdiff_t;

	EntryIterator()
	:	input{nullptr} {}

	EntryIterator(std::istream& in)
	:	input{&in} { setup(); next(); }

	Issues<SourceLocation> issues() const { return issueLocations; }

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

static_assert(std::forward_iterator<EntryIterator<Plain>>);
static_assert(std::forward_iterator<EntryIterator<Json>>);
static_assert(std::forward_iterator<EntryIterator<Rtf>>);

std::optional<Dictionary> parseDictionary(std::istream&, FileType=NoFileType);

} // namespace steno
