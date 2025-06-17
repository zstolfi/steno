#include "steno_parsers.hh"
#include <boost/parser/parser.hpp>

namespace bp = boost::parser;

namespace grammar {
	bp::rule<struct stroke, steno::Stroke>  stroke    = "steno stroke";
	// STKPWHR AO*EU FRPBLGTSDZ
	bp::rule<struct left     , std::string> left      = "left-hand consonants";
	bp::rule<struct middle   , std::string> middle    = "vowel or asterisk";
	bp::rule<struct right    , std::string> right     = "right-hand consonants";
	// 12K3W4R 50*EU 6R7B8G9SDZ
	bp::rule<struct leftNum  , std::string> leftNum   = "left-hand with number bar";
	bp::rule<struct middleNum, std::string> middleNum = "vowel or asterisk with number bar";
	bp::rule<struct rightNum , std::string> rightNum  = "right-hand with number bar";
	bp::rule<struct digitSeq , std::string> digitSeq  = "digit sequence";
	const auto asString = bp::attr(std::string {});
	const auto hash = bp::string("#");
	const auto numberKey = bp::char_("1234506789");
	const auto noNum  = &bp::char_ >> *(bp::char_ - numberKey) >> bp::eoi;
	const auto anyNum = &bp::char_ >> numberKey >> *bp::char_  >> bp::eoi;
	const auto allNum = &bp::char_ >> *numberKey               >> bp::eoi;

	const auto stroke_def
		= 	bp::raw[ -hash >> (
			  	&allNum >> digitSeq
			| 	&noNum  >> left    >> -(middle    >> right   )
			| 	&anyNum >> leftNum >> -(middleNum >> rightNum)
		)]
	;

	const auto left_def
		= 	asString
		>>	-bp::char_('S')
		>>	-bp::char_('T') >> -bp::char_('K')
		>>	-bp::char_('P') >> -bp::char_('W')
		>>	-bp::char_('H') >> -bp::char_('R')
	;

	const auto middle_def
		= 	asString >> bp::char_('-')
		| 	asString >> &bp::char_("AO*EU")
		>>	-bp::char_('A') >> -bp::char_('O')
		>>	-bp::char_('*')
		>>	-bp::char_('E') >> -bp::char_('U')
	;

	const auto right_def
		= 	asString
		>>	-bp::char_('F') >> -bp::char_('R')
		>>	-bp::char_('P') >> -bp::char_('B')
		>>	-bp::char_('L') >> -bp::char_('G')
		>>	-bp::char_('T') >> -bp::char_('S')
		>>	-bp::char_('D') >> -bp::char_('Z')
	;

	const auto leftNum_def
		= 	asString
		>>	-bp::char_('1')
		>>	-bp::char_('2') >> -bp::char_('K')
		>>	-bp::char_('3') >> -bp::char_('W')
		>>	-bp::char_('4') >> -bp::char_('R')
	;

	const auto middleNum_def
		= 	asString >> bp::char_('-')
		| 	asString >> &bp::char_("50*EU")
		>>	-bp::char_('5') >> -bp::char_('0')
		>>	-bp::char_('*')
		>>	-bp::char_('E') >> -bp::char_('U')
	;

	const auto rightNum_def
		= 	asString
		>>	-bp::char_('6') >> -bp::char_('R')
		>>	-bp::char_('7') >> -bp::char_('B')
		>>	-bp::char_('8') >> -bp::char_('G')
		>>	-bp::char_('9') >> -bp::char_('S')
		>>	-bp::char_('D') >> -bp::char_('Z')
	;

	const auto digitSeq_def
		= 	&bp::char_("1234506789") >> asString
		>>	-bp::char_('1') >> -bp::char_('2')
		>>	-bp::char_('3') >> -bp::char_('4')
		>>	-bp::char_('5') >> -bp::char_('0')
		>>	-bp::char_('6') >> -bp::char_('7')
		>>	-bp::char_('8') >> -bp::char_('9')
	;

	BOOST_PARSER_DEFINE_RULES(
		stroke,
		left, middle, right,
		leftNum, middleNum, rightNum,
		digitSeq
	);
}

namespace steno {

// For testing purposes:
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
