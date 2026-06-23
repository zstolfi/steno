#pragma once
#include "steno.hh"
#include <iostream>
#include <optional>

namespace steno {

using ParserInput = std::istream;

enum FileType {
	NoFileType,
	Plain, Json, Rtf,
};

template <FileType FT>
class EntryIterator {
	ParserInput* input {};
	Brief current {};

	struct PlainState {};
	struct JsonState {};
	struct RtfState { enum { Header, Body, Final } value {Header}; };
	using State =
		std::conditional_t<FT == Plain, PlainState,
		std::conditional_t<FT == Json, JsonState,
		std::conditional_t<FT == Rtf, RtfState,
	void>>>;

	State state {};
	void next();

public:
	using value_type = Brief;
	using difference_type = std::ptrdiff_t;

	EntryIterator()
	:	input{nullptr} {}

	EntryIterator(ParserInput& in)
	:	input{&in} { next(); }

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

	void fail(std::string message={}) {
		if (!message.empty()) std::cerr << message << "\n";
		input = nullptr;
	}

	bool over() const {
		return input == nullptr;
	}

	static constexpr bool isWhitespace(char c) {
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}
};

static_assert(std::forward_iterator<EntryIterator<Plain>>);
static_assert(std::forward_iterator<EntryIterator<Json>>);
static_assert(std::forward_iterator<EntryIterator<Rtf>>);

std::optional<Dictionary> parseDictionary(ParserInput&, FileType=NoFileType);

} // namespace steno
