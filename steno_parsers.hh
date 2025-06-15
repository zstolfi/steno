#pragma once
#include "steno.hh"
#include <optional>
#include <span>

//#include "IO/import_PlainText.hh"

namespace steno {
	using ParserInput = std::span<const uint8_t>;
	using ParserDictionaryFn = std::optional<Dictionary> (ParserInput);

	std::optional<Dictionary> parsePlain(ParserInput);
	std::optional<Dictionary> parseJSON(ParserInput);
	std::optional<Dictionary> parseRTF(ParserInput);
	std::optional<Dictionary> parseGuess(ParserInput);

}
