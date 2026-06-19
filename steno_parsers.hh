#pragma once
#include "steno.hh"
#include <iostream>
#include <optional>

namespace steno {

using ParserInput = std::istream;
using ParserDictionaryFn = std::optional<Dictionary> (ParserInput&);

enum FileType {
	NoFileType,
	Plain, JSON, RTF,
};

// TODO: Split into derived classes.
class EntryIterator {
	ParserInput* input {};
	FileType type {};
	Brief current {};

	void next();

public:
	using value_type = Brief;
	using difference_type = std::ptrdiff_t;

	EntryIterator()
	:	input{nullptr} {}
	EntryIterator(ParserInput& in, FileType ft)
	:	input{&in}, type{checkType(ft)} { next(); }

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

	FileType checkType(FileType ft) const {
		assert(ft != NoFileType);
		return ft;
	}

	std::string parseStringJSON();

	enum { rtfHeader, rtfBody, rtfFinal } rtfState {rtfHeader};
};

static_assert(std::forward_iterator<EntryIterator>);

std::optional<Dictionary> parseDictionary(ParserInput&, FileType=NoFileType);

} // namespace steno
