#pragma once
#include "steno.hh"
#include <optional>
#include <span>

//#include "IO/import_PlainText.hh"

namespace steno {
	using ParserInput = std::span<const char>;
	using ParserDictionaryFn = std::optional<Dictionary> (ParserInput);

	// For testing purposes:
	std::optional<Stroke> parseStroke(ParserInput);

	std::optional<Dictionary> parsePlain(ParserInput);
	std::optional<Dictionary> parseJSON(ParserInput);
	std::optional<Dictionary> parseRTF(ParserInput);
	std::optional<Dictionary> parseGuess(ParserInput);

}
