#include "steno_parsers.hh"
#include <boost/parser/parser.hpp>
namespace bp = boost::parser;

/* ~~ Common Parsers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

bp::rule<struct stroke , steno::Stroke > stroke   = "steno stroke";
bp::rule<struct strokes, steno::Strokes> strokes  = "stroke sequence";
// STKPWHR AO*EU FRPBLGTSDZ
bp::rule<struct left     , std::string> left      = "left-hand consonants";
bp::rule<struct middle   , std::string> middle    = "vowel or asterisk";
bp::rule<struct right    , std::string> right     = "right-hand consonants";
// 12K3W4R 50*EU 6R7B8G9SDZ
bp::rule<struct leftNum  , std::string> leftNum   = "left-hand with number bar";
bp::rule<struct middleNum, std::string> middleNum = "vowel or asterisk with number bar";
bp::rule<struct rightNum , std::string> rightNum  = "right-hand with number bar";
bp::rule<struct digitSeq , std::string> digitSeq  = "digit sequence";
// Small helper parsers
const auto asString = bp::attr(std::string {});
const auto hash = bp::string("#");
const auto numberKey = bp::char_("1234506789");
const auto noNum  = &bp::char_ >> *(bp::char_ - numberKey) >> bp::eoi;
const auto anyNum = *bp::char_ >> numberKey >> *bp::char_  >> bp::eoi;
const auto allNum = &bp::char_ >> *numberKey               >> bp::eoi;

const auto stroke_def
	= 	bp::string_view[ -hash >> (
	  	  	&allNum >> digitSeq
	  	| 	&noNum  >> left    >> -(middle    >> right   )
	  	| 	&anyNum >> leftNum >> -(middleNum >> rightNum)
	  	// TODO: Allow "mixed" strokes, ie. 1-TSDZ
	)]
;

const auto strokes_def
	= 	bp::string_view[ stroke % '/' ]
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
	= 	asString >> &bp::char_("1234506789")
	>>	-bp::char_('1') >> -bp::char_('2')
	>>	-bp::char_('3') >> -bp::char_('4')
	>>	-bp::char_('5') >> -bp::char_('0')
	>>	-bp::char_('6') >> -bp::char_('7')
	>>	-bp::char_('8') >> -bp::char_('9')
;

BOOST_PARSER_DEFINE_RULES(
	stroke, strokes,
	left, middle, right,
	leftNum, middleNum, rightNum,
	digitSeq
);

/* ~~ Plain-Text File Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace plain {

using Entry = std::pair<steno::Strokes, std::string>;
bp::rule<struct file, steno::Dictionary> file = "plain-text file";
bp::rule<struct line, Entry            > line = "brief entry";

const auto file_def
	= 	line % bp::eol
;

const auto line_def
	= 	strokes
	>>	'='
	>>	bp::lexeme[ *(bp::char_ - bp::eol) ]
;

BOOST_PARSER_DEFINE_RULES(file, line);

} // namespace plain

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace steno {

std::optional<Dictionary> parsePlain(ParserInput input) {
	return bp::parse(input, plain::file, bp::ws);
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
