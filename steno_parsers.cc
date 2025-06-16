#include "steno_parsers.hh"
#include <boost/parser/parser.hpp>

namespace bp = boost::parser;

namespace grammar {
	const auto left
		= 	-bp::char_('S')
		>>	-bp::char_('T') >> -bp::char_('K')
		>>	-bp::char_('P') >> -bp::char_('W')
		>>	-bp::char_('H') >> -bp::char_('R')
	;

	const auto middle
		= 	bp::char_('-')
		| 	&bp::char_("AO*EU")
		>>	-bp::char_('A') >> -bp::char_('O')
		>>	-bp::char_('*')
		>>	-bp::char_('E') >> -bp::char_('U')
	;

	const auto right
		= 	-bp::char_('F') >> -bp::char_('R')
		>>	-bp::char_('P') >> -bp::char_('B')
		>>	-bp::char_('L') >> -bp::char_('G')
		>>	-bp::char_('T') >> -bp::char_('S')
		>>	-bp::char_('D') >> -bp::char_('Z')
	;

	const auto stroke
		= 	left >> middle >> right
	;
}

namespace steno {

std::optional<Stroke> parseStroke(steno::ParserInput input) {
	if (bp::parse(input, grammar::stroke)) return steno::Stroke {input};
	else return {};
}

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
