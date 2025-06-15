#include "steno_parsers.hh"
#include <boost/parser/parser.hpp>

namespace bp = boost::parser;

namespace steno {

std::optional<Dictionary> parsePlain(ParserInput) {
	return Dictionary {{{"PHRAEUPB"}, "plain"}};
}

std::optional<Dictionary> parseJSON(ParserInput) {
	return Dictionary {{{{"SKWR/SOPB"}, "JSON"}}};
}

std::optional<Dictionary> parseRTF(ParserInput) {
	return Dictionary {{{{"R*/T*/TP*"}, "RTF"}}};
}

std::optional<Dictionary> parseGuess(ParserInput) {
	return Dictionary {{{"TKPWES"}, "guess"}};
}

} // namespace steno
